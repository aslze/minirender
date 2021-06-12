
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
		2*n/(r-l), 0, (r+l)/(r-l), 0,
		0, 2*n/(t-b), (t+b)/(t-b), 0,
		0, 0, -(f+n)/(f-n), -2*f*n/(f-n),
		0, 0, -1, 0);
}

Matrix4 projectionFrustum(float fov, float aspect, float n, float f)
{
	float t = tan(fov / 2);
	return projectionPerspective(-n*t*aspect, n*t * aspect, -n * t, n * t, n, f);
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
	Vec3 ndc[3];
	for (int i = 0; i < 3; i++)
	{
		vertices[i].position = _modelview * vertices[i].position;
		vertices[i].normal = _normal * vertices[i].normal;
		ndc[i] = (_projection * asl::Vec4(vertices[i].position, 1.0f)).h2c();
	}

	if(ndc[0].z < -1 || ndc[1].z < -1 || ndc[2].z < -1) // near clip (should clip into 0, 1 or 2 triangles)
		return;
	
	for(int i = 0; i < 3; i++) // to pixel coords
	{
		ndc[i].x = (ndc[i].x + 1) * _image.cols() / 2;
		ndc[i].y = (ndc[i].y + 1) * _image.rows() / 2;
	}

	Vec2 p[3] = {ndc[0].xy(), ndc[1].xy(), ndc[2].xy()};

	Vec2 pmin = min(p[0], p[1], p[2]);
	Vec2 pmax = max(p[0], p[1], p[2]);

	if (pmax.x < 0 || pmax.y < 0 || pmin.x > _image.cols() || pmin.y > _image.rows())
		return;

	float zz[3] = {-vertices[0].position.z, -vertices[1].position.z, -vertices[2].position.z};
	float a0 = -((p[0]-p[1])^(p[2]-p[1]) / 2);
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

	Vec3 normal01 = vertices[1].normal - vertices[0].normal;
	Vec3 normal02 = vertices[2].normal - vertices[0].normal;

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
	
			float z = zz[0] + k1 * (zz[1]-zz[0]) + k2 * (zz[2] - zz[0]);

			auto& pixdepth = _depth(y, x);
			if (z < pixdepth)
			{
				pixdepth = z;

				// perspective-correct (?)
				float iz = 1 / zz[0] + k1 * (1 / zz[1] - 1 / zz[0]) + k2 * (1 / zz[2] - 1 / zz[0]);
				z = 1 / iz;
				if (fabs(zz[1] - zz[0]) < 0.00001f)
					k1 = (z - zz[0]) / (zz[1] - zz[0]);
				if (fabs(zz[2] - zz[0]) < 0.00001f)
					k2 = (z - zz[0]) / (zz[2] - zz[0]);

				Vec3 normal = vertices[0].normal + k1 * normal01 + k2 * normal02;
				Vec3 position = vertices[0].position + k1 * (vertices[1].position - vertices[0].position) + k2 * (vertices[2].position - vertices[0].position);
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
	_normal = _modelview.inverse().t();
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

void saveImage(const asl::Array2<asl::Vec3>& image, const asl::String& filename)
{
	File file(filename, File::WRITE);
	String header;
	header << "P6\n" << image.cols() << " " << image.rows() << "\n" << 255 << "\n";
	file << header;
	Array<byte> data(image.cols() * 3);

	for (int i = image.rows() - 1; i >= 0; i--)
	{
		for (int j = 0; j < image.cols(); j++)
		{
			Vec3 value = image(i, j) * 255.0f;
			data[j * 3]     = (byte)clamp(value.x, 0.0f, 255.0f);
			data[j * 3 + 1] = (byte)clamp(value.y, 0.0f, 255.0f);
			data[j * 3 + 2] = (byte)clamp(value.z, 0.0f, 255.0f);
		}
		file.write(data.ptr(), data.length());
	}
}

asl::Array2<asl::Vec3> loadImage(const asl::String& filename)
{
	asl::Array2<asl::Vec3> image;
	File file(filename, File::READ);
	if (!file)
		return image;
	String header;
	int nl = 0;
	do {
		char c = file.read<char>();
		if (c == '\n')
			nl++;
		header << c;
	} while (nl < 3 && !file.end());

	Array<String> parts = header.split();

	if (parts.length() != 4 || parts[0] != "P6")
		return image;
	image.resize(parts[2], parts[1]);

	Array<byte> data(image.cols() * 3);

	for (int i = image.rows() - 1; i >= 0; i--)
	{
		int n = file.read(data.ptr(), data.length());
		if (n < data.length())
			break;
		for (int j = 0; j < image.cols(); j++)
		{
			float r = data[j * 3];
			float g = data[j * 3 + 1];
			float b = data[j * 3 + 2];
			image(i, j) = Vec3(r, g, b) / 255.0f;
		}
	}

	return image;
}


asl::Array2<asl::Vec3> SWRenderer::getImage()
{
	return _image;
}
