#include "minirender/Renderer.h"
#include <asl/TextFile.h>

using namespace asl;

namespace minirender {

Matrix4 projectionOrtho(float l, float r, float b, float t, float n, float f)
{
	return Matrix4(
		2 / (r - l), 0, 0, -(r + l) / (r - l),
		0, 2 / (t - b), 0, -(t + b) / (t - b),
		0, 0, -2 / (f - n), -(f + n) / (f - n),
		0, 0, 0, 1);
}

Matrix4 projectionPerspective(float l, float r, float b, float t, float n, float f)
{
	return Matrix4(
		2 * n / (r - l), 0, (r + l) / (r - l), 0,
		0, 2 * n / (t - b), (t + b) / (t - b), 0,
		0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
		0, 0, -1, 0);
}

Matrix4 projectionCV(const Matrix4& K, float w, float h, float n, float f)
{
	return Matrix4(
		K(0, 0) * 2 / w, K(0, 1) * 2 / w, -2 * K(0, 2) / w + 1, 0,
		K(1, 0) * 2 / h, K(1, 1) * 2 / h, 2 * K(1, 2) / h - 1, 0,
		0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
		0, 0, -1, 0
	);
}

Matrix4 projectionFrustum(float fov, float aspect, float n, float f)
{
	float t = tan(fov / 2);
	return projectionPerspective(-n * t * aspect, n * t * aspect, -n * t, n * t, n, f);
}

Matrix4 projectionFrustumH(float fov, float aspect, float n, float f)
{
	float t = tan(fov / 2);
	return projectionPerspective(-n * t, n * t, -n * t / aspect, n * t / aspect, n, f);
}

Matrix4 projectionOrtho(float fov, float aspect, float n, float f)
{
	return projectionOrtho(-fov * aspect / 2, fov * aspect / 2, -fov / 2, fov / 2, n, f);
}

Renderer::Renderer()
{
	setSize(800, 600);
	_projection = projectionOrtho(-40, 40, -30, 30, 50, 120);
	_lightdir = Vec3(-0.15f, 0.6f, 1).normalized();
	_ambient = 0.1f;
	_material = new Material();
	//_material->shininess = 15.0f;
	_scene = nullptr;
	_lighting = true;
	_texturing = true;
}

void Renderer::setSize(int w, int h)
{
	_image.resize(h, w);
	_depth.resize(h, w);
	_points.resize(h, w);
	clear();
}

void Renderer::setScene(Scene* scene)
{
	_scene = scene;
}

void Renderer::clear()
{
	for (int i = 0; i < _image.rows(); i++)
		for (int j = 0; j < _image.cols(); j++)
		{
			_image(i, j) = Vec3(0, 0, 0);
			_depth(i, j) = 1e9f;
			_points(i, j) = Vec3(0, 0, 0);
		}
}

inline Vertex clip(float z, const Vertex& v1, const Vertex& v2)
{
	float k = (fabs(v2.position.z - v1.position.z) < 1e-6f) ? 0.5f : (z - v1.position.z) / (v2.position.z - v1.position.z);
	Vertex v;
	v.position = k * v2.position + (1 - k) * v1.position;
	v.normal = k * v2.normal + (1 - k) * v1.normal;
	v.uv = k * v2.uv + (1 - k) * v1.uv;
	return v;
}

void Renderer::clipTriangle(float z, Vertex v[3])
{
	if (v[0].position.z > z && v[1].position.z > z && v[2].position.z > z)
		return;

	while (v[0].position.z < v[1].position.z || v[0].position.z < v[2].position.z)
	{
		swap(v[0], v[1]);
		swap(v[0], v[2]);
	}

	if (v[1].position.z > z)
	{
		Vertex v02 = clip(z, v[0], v[2]);
		Vertex v12 = clip(z, v[1], v[2]);
		paintTriangle(v02, v12, v[2], false);
	}
	else if (v[2].position.z > z)
	{
		Vertex v01 = clip(z, v[0], v[1]);
		Vertex v12 = clip(z, v[1], v[2]);
		paintTriangle(v01, v[1], v12, false);
	}
	else
	{
		Vertex v01 = clip(z, v[0], v[1]);
		Vertex v02 = clip(z, v[0], v[2]);
		paintTriangle(v01, v[1], v[2], false);
		paintTriangle(v01, v[2], v02, false);
	}
}

void Renderer::paintTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, bool world)
{
	Vec3 vertices[3] = { v0.position, v1.position, v2.position };
	Vec3 normals[3] = { v0.normal, v1.normal, v2.normal };
	Vec2 texcoords[3] = { v0.uv, v1.uv, v2.uv };
	Vec4 cverts[3];
	Vec3 ndc[3];
	if (world)
		for (int i = 0; i < 3; i++)
		{
			vertices[i] = _modelview * vertices[i];
			normals[i] = _normalmat * normals[i];
		}

	if (world && (vertices[0].z > _znear || vertices[1].z > _znear || vertices[2].z > _znear))
	{
		if (vertices[0].z > _znear && vertices[1].z > _znear && vertices[2].z > _znear)
			return;

		Vertex verts[3] = { Vertex(vertices[0], normals[0], texcoords[0]), Vertex(vertices[1], normals[1], texcoords[1]), Vertex(vertices[2], normals[2], texcoords[2]) };
		clipTriangle(_znear, verts);
		return;
	}

	for (int i = 0; i < 3; i++)
	{
		cverts[i] = _projection * asl::Vec4(vertices[i], 1.0f);
		ndc[i] = cverts[i].h2c();
	}

	for (int i = 0; i < 3; i++) // pixel coords
	{
		ndc[i].x = (1 + ndc[i].x) * _image.cols() / 2;
		ndc[i].y = (1 - ndc[i].y) * _image.rows() / 2;
	}

	Vec2 p[3] = { ndc[0].xy(), ndc[1].xy(), ndc[2].xy() };

	Vec2 pmin = min(p[0], p[1], p[2]);
	Vec2 pmax = max(p[0], p[1], p[2]);

	if (pmax.x < 0 || pmax.y < 0 || pmin.x > _image.cols() || pmin.y > _image.rows())
		return;

	float a = -((p[0] - p[1]) ^ (p[2] - p[1]) / 2);
	float i2a = (a == 0) ? 0.0f : 1.0f / (2 * a);

	if (a >= 0) // front face
	{
		return; // back face cull
	}

	Vec2 n0 = (p[2] - p[1]).perpend() * i2a;
	Vec2 n1 = (p[0] - p[2]).perpend() * i2a;
	Vec2 n2 = (p[1] - p[0]).perpend() * i2a;

	pmin.x = (float)clamp((int)pmin.x, 0, _image.cols() - 1);
	pmax.x = (float)clamp((int)pmax.x, 0, _image.cols() - 1);
	pmin.y = (float)clamp((int)pmin.y, 0, _image.rows() - 1);
	pmax.y = (float)clamp((int)pmax.y, 0, _image.rows() - 1);

	float zz[4] = { ndc[0].z, ndc[1].z, ndc[2].z, 1 };
	float iz[4] = { 1 / -vertices[0].z, 1 / -vertices[1].z, 1 / -vertices[2].z, 1 };

	bool persp = _projection(3, 3) == 0;
	bool hasspecular = _material->shininess != 0;
	bool hastexture = _texturing && _material->texture.rows() > 0;

	Vec3 color = _material->diffuse;

	float k[4] = { 0, 0, 0, 0 };

	for (float y = floor(pmin.y); y <= pmax.y; y++)
	{
		Vec2 pt(floor(pmin.x), y);
		float e1 = n1 * (pt - p[2]);
		float e2 = n2 * (pt - p[0]);
		for (float x = floor(pmin.x); x <= pmax.x; x++, e1 += n1.x, e2 += n2.x)
		{
			if (e1 < 0 || e2 < 0 || 1 - e1 - e2 < 0)
				continue;
			k[0] = 1 - e1 - e2;
			k[1] = e1;
			k[2] = e2;

			float z = k[0] * zz[0] + k[1] * zz[1] + k[2] * zz[2] + k[3] * zz[3];

			int i = int(y), j = int(x);

			auto& pixdepth = _depth(i, j);
			if (z < pixdepth)
			{
				pixdepth = z;
				if (persp)
				{
					z = 1 / (k[0] * iz[0] + k[1] * iz[1] + k[2] * iz[2]);
					k[0] *= z * iz[0];
					k[1] *= z * iz[1];
					k[2] *= z * iz[2];
				}

				Vec3 position = k[0] * vertices[0] + k[1] * vertices[1] + k[2] * vertices[2];

				if (hastexture)
				{
					Vec2 uv = k[0] * texcoords[0] + k[1] * texcoords[1] + k[2] * texcoords[2];
					color = _material->texture(int(fract(uv.y) * _material->texture.rows()), int(fract(uv.x) * _material->texture.cols()));
				}
				
				Vec3 value = _material->emissive;
				
				if (_lighting)
				{
					Vec3 normal = (k[0] * normals[0] + k[1] * normals[1] + k[2] * normals[2]).normalized();
					value += (max(0.0f, normal * _lightdir) + _ambient) * color;
					if (hasspecular)
					{
						Vec3 viewDir = -position.normalized();
						float specular = pow(max((_lightdir + viewDir).normalized() * normal, 0.0f), _material->shininess);
						value += specular * _material->specular;
					}
				}
				_image(i, j) = value;
				_points(i, j) = position;
			}
		}
	}
}

void Renderer::render()
{
	clear();
	_renderables.clear();
	_scene->collectShapes(_renderables, Matrix4::identity());

	_ambient = _scene->ambientLight;

	_znear = _projection(3, 2) != 0 ? _projection(2, 3) / (_projection(2, 2) - 1) : (_projection(2, 3) + 1) / _projection(2, 2);

	for (auto& item : _renderables)
	{
		paintMesh(item.mesh, item.transform);
	}

	for (int i = 0; i < _depth.rows(); i++)
		for (int j = 0; j < _depth.cols(); j++)
			if (_depth(i, j) > 1.0f)
				_points(i, j) = Vec3(0, 0, 0);
}

void Renderer::paintMesh(TriMesh* mesh, const Matrix4& transform)
{
	if (mesh->material)
		_material = mesh->material;
	_modelview = _view * transform;
	_normalmat = _modelview.inverse().t();
	for (int i = 0; i < mesh->indices.length(); i += 3)
	{
		int ia = mesh->indices[i];
		int ib = mesh->indices[i + 1];
		int ic = mesh->indices[i + 2];
		Vec3 a = mesh->vertices[ia];
		Vec3 b = mesh->vertices[ib];
		Vec3 c = mesh->vertices[ic];
		Vec3 na = mesh->normals[mesh->normalsI[i]];
		Vec3 nb = mesh->normals[mesh->normalsI[i + 1]];
		Vec3 nc = mesh->normals[mesh->normalsI[i + 2]];
		if (mesh->texcoords.length() > 0)
		{
			Vec2 ta = mesh->texcoords[mesh->texcoordsI[i]];
			Vec2 tb = mesh->texcoords[mesh->texcoordsI[i + 1]];
			Vec2 tc = mesh->texcoords[mesh->texcoordsI[i + 2]];
			paintTriangle(Vertex(a, na, ta), Vertex(b, nb, tb), Vertex(c, nc, tc));
		}
		else
			paintTriangle(Vertex(a, na), Vertex(b, nb), Vertex(c, nc));
	}
}

asl::Array2<asl::Vec3> Renderer::getImage()
{
	return _image;
}

}
