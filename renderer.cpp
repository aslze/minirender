
#include "renderer.h"
#include <asl/TextFile.h>

using namespace asl;

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

Matrix4 projectionFrustum(float fov, float aspect, float n, float f)
{
	float t = tan(fov / 2);
	return projectionPerspective(-n*t*aspect, n*t * aspect, -n * t, n * t, n, f);
}

Matrix4 projectionOrtho(float fov, float aspect, float n, float f)
{
	return projectionOrtho(-fov*aspect/2, fov*aspect/2, -fov/2, fov/2, n, f);
}


SWRenderer::SWRenderer()
{
	setSize(800, 600);
	_projection = projectionOrtho(-40, 40, -30, 30, 50, 120);
	_light = Vec3(-0.15f, 0.6f, 1).normalized();
	_color = Vec3(1, 0, 0.2f);
	_ambient = Vec3(0.1f, 0.1f, 0.1f);
	_specular = Vec3(0.8f, 0.8f, 0.8f);
	_shininess = 12.0f;
}

void SWRenderer::setSize(int w, int h)
{
	_image.resize(h, w);
	_depth.resize(h, w);
	_points.resize(h, w);
	clear();
}

void SWRenderer::setScene(TriMesh* part)
{
	_object = part;
}

void SWRenderer::clear()
{
	for (int i = 0; i < _image.rows(); i++)
		for (int j = 0; j < _image.cols(); j++)
		{
			_image(i, j) = Vec3(0, 0, 0);
			_depth(i, j) = 1e9f;
			_points(i, j) = Vec3(0, 0, 0);
		}
}


void SWRenderer::paintTriangle(const Vertex& a, const Vertex& b, const Vertex& c)
{
	Vertex vertices[3] = {a, b, c};
	Vec4 cverts[3];
	Vec3 ndc[3];
	for (int i = 0; i < 3; i++)
	{
		vertices[i].position = _modelview * vertices[i].position;
		vertices[i].normal = _normalmat * vertices[i].normal;
		cverts[i] = _projection * asl::Vec4(vertices[i].position, 1.0f);
		ndc[i] = cverts[i].h2c();
	}

	if(ndc[0].z < -1 || ndc[1].z < -1 || ndc[2].z < -1) // near clip (should clip into 0, 1 or 2 triangles)
		return;
	
	for(int i = 0; i < 3; i++) // pixel coords
	{
		ndc[i].x = (ndc[i].x + 1) * _image.cols() / 2;
		ndc[i].y = (ndc[i].y + 1) * _image.rows() / 2;
	}

	Vec2 p[3] = {ndc[0].xy(), ndc[1].xy(), ndc[2].xy()};

	Vec2 pmin = min(p[0], p[1], p[2]);
	Vec2 pmax = max(p[0], p[1], p[2]);

	if (pmax.x < 0 || pmax.y < 0 || pmin.x > _image.cols() || pmin.y > _image.rows())
		return;

	float a0 = -((p[0] - p[1]) ^ (p[2] - p[1]) / 2);
	float i2a = 1.0f / (2 * a0);
	if(a0 == 0)
		i2a = 0;

	if (a0 < 0) // front face
	{
		return; // back face cull
	}

	Vec2 n0 = (p[2] - p[1]).perpend();
	Vec2 n1 = (p[0] - p[2]).perpend();
	Vec2 n2 = (p[1] - p[0]).perpend();

	float a2 = 2.0f * a0;

	pmin.x = (float)clamp((int)pmin.x, 0, _image.cols()-1);
	pmax.x = (float)clamp((int)pmax.x, 0, _image.cols()-1);
	pmin.y = (float)clamp((int)pmin.y, 0, _image.rows()-1);
	pmax.y = (float)clamp((int)pmax.y, 0, _image.rows()-1);

	float zz[] = { ndc[0].z, ndc[1].z, ndc[2].z };
	float iz[] = { 1 / -vertices[0].position.z, 1 / -vertices[1].position.z, 1 / -vertices[2].position.z };

	bool persp = _projection(3, 3) == 0;

	for(float y = floor(pmin.y); y <= pmax.y; y++)
	{
		Vec2 pt(floor(pmin.x), y);
		float e1 = n1 * (pt - p[2]);
		float e2 = n2 * (pt - p[0]);
		for(float x = floor(pmin.x); x <= pmax.x; x++, e1 += n1.x, e2 += n2.x)
		{
			if(e1 < 0 || e2 < 0 || a2 - e1 - e2 < 0)
				continue;
			float k1 = e1 * i2a;
			float k2 = e2 * i2a;
			float k0 = 1 - k1 - k2;
	
			float z = k0 * zz[0] + k1 * zz[1] + k2 * zz[2];

			auto& pixdepth = _depth(y, x);
			if (z < pixdepth)
			{
				pixdepth = z;
				float k0 = 1 - k1 - k2;
				if (persp)
				{
					z = 1 / (k0 * iz[0] + k1 * iz[1] + k2 * iz[2]);
					k0 *= z * iz[0];
					k1 *= z * iz[1];
					k2 *= z * iz[2];
				}
				Vec3 normal = k0 * vertices[0].normal + k1 * vertices[1].normal + k2 * vertices[2].normal;
				Vec3 position = k0 * vertices[0].position + k1 * vertices[1].position + k2 * vertices[2].position;
				Vec3 viewDir = -position.normalized();
				float specular = pow(max((_light + viewDir).normalized() * normal, 0.0f), _shininess);
				Vec3 value = max(0.0f, normal.normalized() * _light) * _color + specular * _specular + _ambient;
				_image(y, x) = value;
				_points(y, x) = position;
			}
		}
	}
}

void SWRenderer::render()
{
	clear();
	paintMesh(_object, Matrix4::identity());
}

void SWRenderer::paintMesh(TriMesh* mesh, const Matrix4& transform)
{
	_modelview = _view * transform;
	_normalmat = _modelview.inverse().t();
	for (int i = 0; i < mesh->indices.length(); i += 3)
	{
		int ia = mesh->indices[i];
		int ib = mesh->indices[i+1];
		int ic = mesh->indices[i+2];
		Vec3 a = mesh->vertices[ia];
		Vec3 b = mesh->vertices[ib];
		Vec3 c = mesh->vertices[ic];
		Vec3 na = mesh->normals[mesh->normalsI[i]];
		Vec3 nb = mesh->normals[mesh->normalsI[i+1]];
		Vec3 nc = mesh->normals[mesh->normalsI[i+2]];
		paintTriangle(Vertex(a, na), Vertex(b, nb), Vertex(c, nc));
	}
}

asl::Array2<asl::Vec3> SWRenderer::getImage()
{
	return _image;
}
