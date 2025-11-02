#include <minirender/primitives.h>

namespace minirender
{

asl::Shared<minirender::TriMesh> createCube(float size)
{
	using namespace minirender;
	auto mesh = new TriMesh();

	const float h = size * 0.5f;

	// For flat shading we duplicate vertices per face (6 faces * 4 verts = 24)
	// Face order: +X, -X, +Y, -Y, +Z, -Z
	struct FaceDef
	{
		asl::Vec3 v0, v1, v2, v3, n;
	};
	FaceDef faces[6] = {
		{ asl::Vec3(h, -h, -h), asl::Vec3(h, -h, h), asl::Vec3(h, h, h), asl::Vec3(h, h, -h), asl::Vec3(1, 0, 0) },
		{ asl::Vec3(-h, -h, h), asl::Vec3(-h, -h, -h), asl::Vec3(-h, h, -h), asl::Vec3(-h, h, h), asl::Vec3(-1, 0, 0) },
		{ asl::Vec3(-h, h, -h), asl::Vec3(h, h, -h), asl::Vec3(h, h, h), asl::Vec3(-h, h, h), asl::Vec3(0, 1, 0) },
		{ asl::Vec3(-h, -h, h), asl::Vec3(h, -h, h), asl::Vec3(h, -h, -h), asl::Vec3(-h, -h, -h), asl::Vec3(0, -1, 0) },
		{ asl::Vec3(h, -h, h), asl::Vec3(-h, -h, h), asl::Vec3(-h, h, h), asl::Vec3(h, h, h), asl::Vec3(0, 0, 1) },
		{ asl::Vec3(-h, -h, -h), asl::Vec3(h, -h, -h), asl::Vec3(h, h, -h), asl::Vec3(-h, h, -h), asl::Vec3(0, 0, -1) }
	};

	// texcoords per face (0,0) bottom-left, (1,1) top-right
	asl::Vec2 uv0(0.0f, 0.0f), uv1(1.0f, 0.0f), uv2(1.0f, 1.0f), uv3(0.0f, 1.0f);

	for (int f = 0; f < 6; ++f)
	{
		int base = mesh->vertices.length();

		mesh->vertices << faces[f].v0 << faces[f].v1 << faces[f].v2 << faces[f].v3;
		mesh->normals << faces[f].n << faces[f].n << faces[f].n << faces[f].n;
		mesh->texcoords << uv0 << uv1 << uv2 << uv3;

		mesh->indices << base + 0 << base + 2 << base + 1;
		mesh->normalsI << base + 0 << base + 2 << base + 1;
		mesh->indices << base + 0 << base + 3 << base + 2;
		mesh->normalsI << base + 0 << base + 3 << base + 2;
	}

	mesh->material = new minirender::Material();
	return mesh;
}

asl::Shared<minirender::TriMesh> createCylinder(float radius, float height, int segments, int heightSegments, bool caps)
{
	using namespace minirender;
	auto mesh = new TriMesh();

	if (segments < 3)
		segments = 3;
	if (heightSegments < 1)
		heightSegments = 1;

	const float halfH = height * 0.5f;
	const float twoPi = 2.0f * asl::PIf;

	for (int h = 0; h <= heightSegments; ++h)
	{
		float v = (float)h / (float)heightSegments;
		float y = -halfH + v * height;
		for (int r = 0; r < segments; ++r)
		{
			float u = (float)r / (float)segments;
			float angle = u * twoPi;
			float x = radius * std::cos(angle);
			float z = radius * std::sin(angle);
			mesh->vertices << asl::Vec3(x, y, z);
			// outward normal (per-vertex)
			mesh->normals << asl::Vec3(x, 0.0f, z).normalized();
			mesh->texcoords << asl::Vec2(u, v);
		}
	}

	for (int h = 0; h < heightSegments; ++h)
	{
		for (int r = 0; r < segments; ++r)
		{
			int i0 = h * segments + r;
			int i1 = h * segments + (r + 1) % segments;
			int i2 = (h + 1) * segments + r;
			int i3 = (h + 1) * segments + (r + 1) % segments;

			// Triangles: (i0, i2, i1) and (i1, i2, i3)
			mesh->indices << i0 << i2 << i1;
			mesh->normalsI << i0 << i2 << i1;
			mesh->indices << i1 << i2 << i3;
			mesh->normalsI << i1 << i2 << i3;
		}
	}

	if (caps)
	{
		int bottomRingStart = mesh->vertices.length();
		for (int r = 0; r < segments; ++r)
		{
			int       orig = r;
			asl::Vec3 p = mesh->vertices[orig];
			mesh->vertices << p;
			mesh->normals << asl::Vec3(0.0f, -1.0f, 0.0f);
			mesh->texcoords << asl::Vec2((p.x / radius + 1.0f) * 0.5f, (p.z / radius + 1.0f) * 0.5f);
		}
		int bottomCenter = mesh->vertices.length();
		mesh->vertices << asl::Vec3(0.0f, -halfH, 0.0f);
		mesh->normals << asl::Vec3(0.0f, -1.0f, 0.0f);
		mesh->texcoords << asl::Vec2(0.5f, 0.5f);

		for (int r = 0; r < segments; ++r)
		{
			int a = bottomRingStart + r;
			int b = bottomRingStart + (r + 1) % segments;
			mesh->indices << bottomCenter << a << b;
			mesh->normalsI << bottomCenter << a << b;
		}

		int topOrigStart = heightSegments * segments; // original top ring first index
		int topRingStart = mesh->vertices.length();
		for (int r = 0; r < segments; ++r)
		{
			int       orig = topOrigStart + r;
			asl::Vec3 p = mesh->vertices[orig];
			mesh->vertices << p;
			mesh->normals << asl::Vec3(0.0f, 1.0f, 0.0f);
			mesh->texcoords << asl::Vec2((p.x / radius + 1.0f) * 0.5f, (p.z / radius + 1.0f) * 0.5f);
		}
		int topCenter = mesh->vertices.length();
		mesh->vertices << asl::Vec3(0.0f, halfH, 0.0f);
		mesh->normals << asl::Vec3(0.0f, 1.0f, 0.0f);
		mesh->texcoords << asl::Vec2(0.5f, 0.5f);

		for (int r = 0; r < segments; ++r)
		{
			int a = topRingStart + r;
			int b = topRingStart + (r + 1) % segments;
			mesh->indices << topCenter << b << a;
			mesh->normalsI << topCenter << b << a;
		}
	}

	mesh->material = new minirender::Material();

	return mesh;
}

asl::Shared<minirender::TriMesh> createSphere(float radius, int latSegments, int longSegments)
{
	using namespace minirender;
	auto mesh = new TriMesh();

	if (latSegments < 2)
		latSegments = 2;
	if (longSegments < 3)
		longSegments = 3;

	mesh->vertices << asl::Vec3(0.0f, radius, 0.0f);
	mesh->normals << asl::Vec3(0.0f, 1.0f, 0.0f);
	mesh->texcoords << asl::Vec2(0.5f, 0.0f);

	int firstRingStart = mesh->vertices.length();
	for (int lat = 1; lat < latSegments; ++lat)
	{
		float v = (float)lat / (float)latSegments; // 0..1
		float phi = v * asl::PIf;                  // 0..pi
		float sinPhi = std::sin(phi);
		float cosPhi = std::cos(phi);
		for (int lon = 0; lon < longSegments; ++lon)
		{
			float u = (float)lon / (float)longSegments; // 0..1
			float theta = u * 2.0f * asl::PIf;          // 0..2pi
			float x = radius * sinPhi * std::cos(theta);
			float y = radius * cosPhi;
			float z = radius * sinPhi * std::sin(theta);
			mesh->vertices << asl::Vec3(x, y, z);
			mesh->normals << asl::Vec3(x, y, z).normalized();
			mesh->texcoords << asl::Vec2(u, 1.0f - v); // v flipped
		}
	}

	int bottomIndex = mesh->vertices.length();
	mesh->vertices << asl::Vec3(0.0f, -radius, 0.0f);
	mesh->normals << asl::Vec3(0.0f, -1.0f, 0.0f);
	mesh->texcoords << asl::Vec2(0.5f, 1.0f);

	for (int lon = 0; lon < longSegments; ++lon)
	{
		int i1 = firstRingStart + lon;
		int i2 = firstRingStart + (lon + 1) % longSegments;
		mesh->indices << 0 << i1 << i2;
		mesh->normalsI << 0 << i1 << i2;
	}

	int rings = latSegments - 1;
	if (rings >= 2)
	{
		for (int lat = 0; lat < rings - 1; ++lat)
		{
			int row1 = firstRingStart + lat * longSegments;
			int row2 = row1 + longSegments;
			for (int lon = 0; lon < longSegments; ++lon)
			{
				int a = row1 + lon;
				int b = row1 + (lon + 1) % longSegments;
				int c = row2 + lon;
				int d = row2 + (lon + 1) % longSegments;

				mesh->indices << a << b << c;
				mesh->normalsI << a << b << c;
				mesh->indices << b << d << c;
				mesh->normalsI << b << d << c;
			}
		}
	}

	int lastRingStart = firstRingStart + (rings - 1) * longSegments;
	for (int lon = 0; lon < longSegments; ++lon)
	{
		int i1 = lastRingStart + lon;
		int i2 = lastRingStart + (lon + 1) % longSegments;
		mesh->indices << bottomIndex << i1 << i2;
		mesh->normalsI << bottomIndex << i1 << i2;
	}

	mesh->material = new minirender::Material();
	return mesh;
}

}
