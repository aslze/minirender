#include "minirender/Renderer.h"
#include <asl/Matrix3.h>

#define PREMULT
#define FAST_LIGHT
#define POINTLIGHT

using namespace asl;

namespace minirender {


inline Vec3 htransform(const Matrix4& m, const Vec3& p)
{
	float iw = 1/(m(3, 0) * p.x + m(3, 1) * p.y + m(3, 2) * p.z + m(3, 3));
	return Vec3(
		iw * (m(0, 0) * p.x + m(0, 1) * p.y + m(0, 2) * p.z + m(0, 3)),
		iw * (m(1, 0) * p.x + m(1, 1) * p.y + m(1, 2) * p.z + m(1, 3)),
		iw * (m(2, 0) * p.x + m(2, 1) * p.y + m(2, 2) * p.z + m(2, 3)));
}

inline Vec3 mix(float k[3], const Vec3 v[3])
{
	//float x[4][4] = {
	//	{ v[0].x, v[0].y, v[0].z, 0 }, { v[1].x, v[1].y, v[1].z, 0 }, { v[2].x, v[2].y, v[2].z, 0 }, { 0, 0, 0, 0 }
	//};
	float x[4][4] = {
			{ v[0].x, v[1].x, v[2].x, 0 }, { v[0].y, v[1].y, v[2].y, 0 }, { v[0].z, v[1].z, v[2].z, 0 }, { 0, 0, 0, 0 }
		};
	float y[4] = { 0, 0, 0, 0 };
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
			y[i] += k[j] * x[i][j];
	}

	return Vec3(y[0], y[1], y[2]);
}

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
	_light = Vec3(-0.15f, 0.6f, 1).normalized();
	_ambient = 0.1f;
	_defmaterial = new Material();
	_material = _defmaterial;
	_scene = nullptr;
	_lighting = true;
	_texturing = true;
	_bgcolor = Vec3(0, 0, 0);
	_lightIsPoint = false;
}

void Renderer::setSize(int w, int h)
{
	_image.resize(h, w);
	_depth.resize(h, w);
	_points.resize(h, w);
	_pnormals.resize(h, w);
	clear();
}

void Renderer::setScene(Scene* scene)
{
	_scene = scene;
}

void Renderer::clear()
{
	_image.set(_bgcolor);
	_depth.set(1e11f);
	_points.set(Vec3(0, 0, 0));
	_pnormals.set(Vec3(0, 0, 1));
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

	if (world && (vertices[0].z > _znear || vertices[1].z > _znear || vertices[2].z > _znear))
	{
		if (vertices[0].z > _znear && vertices[1].z > _znear && vertices[2].z > _znear)
			return;

		Vertex verts[3] = { Vertex(vertices[0], normals[0], texcoords[0]), Vertex(vertices[1], normals[1], texcoords[1]), Vertex(vertices[2], normals[2], texcoords[2]) };
		clipTriangle(_znear, verts);
		return;
	}

	float w = (float)_image.cols(), h = (float)_image.rows();

	Vec3 ndc[3];
	Vec2 p[3];
	Vec2 pmin(1e30f, 1e30f);
	Vec2 pmax(-1e30f, -1e30f);

	for (int i = 0; i < 3; i++)
	{
		ndc[i] = htransform(_projection, vertices[i]);
	}

	Matrix3 topx(w / 2, 0, w / 2, 0, -h / 2, h / 2);

	for (int i = 0; i < 3; i++) // pixel coords
	{
		p[i].x = (1 + ndc[i].x) * (w / 2);
		p[i].y = (1 - ndc[i].y) * (h / 2);
		//p[i] = topx * ndc[i].xy();
		pmin = min(pmin, p[i]);
		pmax = max(pmax, p[i]);
	}

	if (pmax.x < 0 || pmax.y < 0 || pmin.x > w || pmin.y > h)
		return;

	float a = (p[0] - p[1]) ^ (p[2] - p[1]); // * 0.5f;

	if (a <= 0) // front face
	{
		return; // back face cull
	}

	float i2a = (a == 0) ? 0.0f : -1.0f / (/*2 * */ a);

	//Vec2 n0 = (p[2] - p[1]).perpend() * i2a;
	Vec2 n1 = (p[0] - p[2]).perpend() * i2a;
	Vec2 n2 = (p[1] - p[0]).perpend() * i2a;

	pmin.x = clamp(pmin.x, 0.f, w - 1);
	pmax.x = clamp(pmax.x, 0.f, w - 1);
	pmin.y = clamp(pmin.y, 0.f, h - 1);
	pmax.y = clamp(pmax.y, 0.f, h - 1);

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

			float z;

			if (persp)
			{
				z = 1 / (k[0] * iz[0] + k[1] * iz[1] + k[2] * iz[2]);
				k[0] *= z * iz[0];
				k[1] *= z * iz[1];
				k[2] *= z * iz[2];
			}
			else
				z = k[0] * zz[0] + k[1] * zz[1] + k[2] * zz[2] + k[3] * zz[3];

			int i = int(y), j = int(x);

			auto& pixdepth = _depth(i, j);

			if (z < pixdepth)
			{
				pixdepth = z;

				//Vec3 position = mix(k, vertices);
				Vec3 position = k[0] * vertices[0] + k[1] * vertices[1] + k[2] * vertices[2];

				if (hastexture)
				{
					Vec2 uv = k[0] * texcoords[0] + k[1] * texcoords[1] + k[2] * texcoords[2];
					color = _material->texture(int(fract(uv.y) * _material->texture.rows()), int(fract(uv.x) * _material->texture.cols()));
				}

				Vec3 value = _material->emissive;

				if (_lighting)
				{
					Vec3 _lightdir = _lightIsPoint ? (this->_lightdir - position).normalized() : this->_lightdir;
					
#ifndef FAST_LIGHT
					Vec3 normal = (k[0] * normals[0] + k[1] * normals[1] + k[2] * normals[2]).normalized();
					value += (max(0.0f, normal * _lightdir) + _ambient) * color;
#else
					Vec3 normal = (k[0] * normals[0] + k[1] * normals[1] + k[2] * normals[2]);
					value += (max(0.0f, normal * _lightdir)/normal.length() + _ambient) * color;
#endif

					if (hasspecular)
					{
						Vec3 viewDir = position.normalized();
#ifndef FAST_LIGHT
						float specular = pow(max((_lightdir - viewDir).normalized() * normal, 0.0f), _material->shininess);
#else
						float specular = pow(max((_lightdir - viewDir) * normal, 0.0f) / ((_lightdir - viewDir).length() * normal.length()), _material->shininess);
#endif
						value += specular * _material->specular;
					}
					//if (!hasspecular)
					_pnormals(i, j) = normal;
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

	if (_lightIsPoint)
		_lightdir = _view * _light;
	else
		_lightdir = _light.normalized(); // this makes the light camera relative!

	_ambient = _scene->ambientLight;

	bool persp = _projection(3, 3) == 0;
	_znear = persp ? _projection(2, 3) / (_projection(2, 2) - 1) : (_projection(2, 3) + 1) / _projection(2, 2);
	float zfar = persp ? _projection(2, 3) / (_projection(2, 2) + 1) : (_projection(2, 3) - 1) / _projection(2, 2);

	_znear = -_znear;

	for (auto& item : _renderables)
	{
		paintMesh(item.mesh, item.transform);
	}

	float fardepth = persp ? zfar : 1.0f;

	for (int i = 0; i < _depth.rows(); i++)
		for (int j = 0; j < _depth.cols(); j++)
			if (_depth(i, j) > fardepth)
				_points(i, j) = Vec3(0, 0, 0);
}

void Renderer::paintMesh(TriMesh* mesh, const Matrix4& transform)
{
	_material = (mesh->material) ? mesh->material : _defmaterial;
	_modelview = _view * transform;
	_normalmat = _modelview.inverse().t();

#ifdef PREMULT
	_vertices.resize(mesh->vertices.length());
	_normals.resize(mesh->normals.length());

	for (int i = 0; i < _vertices.length(); i++)
		_vertices[i] = _modelview * mesh->vertices[i];

	for (int i = 0; i < _normals.length(); i++)
		_normals[i] = _normalmat * mesh->normals[i];
#endif

	for (int i = 0; i < mesh->indices.length(); i += 3)
	{
		int ia = mesh->indices[i];
		int ib = mesh->indices[i + 1];
		int ic = mesh->indices[i + 2];
#ifndef PREMULT
		Vec3 a = _modelview * mesh->vertices[ia];
		Vec3 b = _modelview * mesh->vertices[ib];
		Vec3 c = _modelview * mesh->vertices[ic];
		Vec3 na = _normalmat * mesh->normals[mesh->normalsI[i]];
		Vec3 nb = _normalmat * mesh->normals[mesh->normalsI[i + 1]];
		Vec3 nc = _normalmat * mesh->normals[mesh->normalsI[i + 2]];
#else
		Vec3& a = _vertices[ia];
		Vec3& b = _vertices[ib];
		Vec3& c = _vertices[ic];
		Vec3& na = _normals[mesh->normalsI[i]];
		Vec3& nb = _normals[mesh->normalsI[i + 1]];
		Vec3& nc = _normals[mesh->normalsI[i + 2]];
#endif
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

asl::Array2<asl::Vec3> Renderer::getImage() const
{
	return _image;
}

}
