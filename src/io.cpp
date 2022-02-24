#include <asl/TextFile.h>
#include <asl/StreamBuffer.h>
#include <asl/Map.h>
#include <asl/Path.h>
#include "minirender/io.h"

using namespace asl;

namespace minirender {

TriMesh* loadSTLa(const asl::String& filename)
{
	TextFile file(filename, File::READ);
	if (!file)
		return NULL;

	String tag;
	file >> tag;
	if (tag != "solid")
		return NULL;

	TriMesh* obj = new TriMesh();

	int np = 0, indexv = 0, indexn = 0;

	while (!file.end())
	{
		file >> tag;
		if (tag == "facet")
		{
		}
		else if (tag == "endfacet")
		{
			if (np == 3)
			{
				obj->indices << indexv - 3 << indexv - 2 << indexv - 1;
				obj->normalsI << indexn - 1 << indexn - 1 << indexn - 1;
			}
			np = 0;
		}
		else if (tag == "normal")
		{
			Array<float> a = file.readLine().split_<float>();
			Vec3 v(a[0], a[1], a[2]);
			obj->normals << v;
			indexn++;
		}
		else if (tag == "vertex")
		{
			Array<float> a = file.readLine().split_<float>();
			Vec3 v(a[0], a[1], a[2]);
			obj->vertices << v;
			np++;
			indexv++;
		}
	}
	return obj;
}


TriMesh* loadSTLb(const String& filename)
{
	File file(filename, File::READ);
	if (!file)
		return NULL;

	char s[88];
	if (file.read(s, 80) != 80)
		return NULL;

	int nf = file.read<int>();

	if (nf > 100000000)
		return NULL;

	TriMesh* obj = new TriMesh();

	Array<byte> data(3 * sizeof(float) + 3 * 3 * sizeof(float) + 2);

	obj->normals.reserve(nf);
	obj->vertices.reserve(nf * 3);
	obj->indices.reserve(nf * 3);
	obj->normalsI.reserve(nf * 3);

	asl::Vec3 v;

	int indexn = 0, indexv = 0;
	for (int i = 0; i < nf; i++, indexn++)
	{
		file.read(data.ptr(), data.length());
		StreamBufferReader reader(data);
		reader >> v.x >> v.y >> v.z;
		obj->normals << v;

		for (int j = 0; j < 3; j++, indexv++)
		{
			reader >> v.x >> v.y >> v.z;
			obj->vertices << v;
		}

		obj->indices << indexv - 3 << indexv - 2 << indexv - 1;
		obj->normalsI << indexn << indexn << indexn;

		reader.skip(2);
	}

	return obj;
}

SceneNode* loadMesh(const asl::String& filename)
{
	if (Path(filename).hasExtension("stl"))
	{
		TriMesh* mesh = loadSTL(filename);
		SceneNode* node = new SceneNode;
		node->children << mesh;
		return node;
	}
	else if (Path(filename).hasExtension("obj"))
	{
		return loadOBJ(filename);
	}
	return new SceneNode;
}

TriMesh* loadSTL(const asl::String& filename)
{
	Array<byte> bytes = asl::File(filename).firstBytes(5);
	if (bytes.length() < 5)
		return NULL;

	if (String((char*)bytes.ptr(), 5) == "solid")
		return loadSTLa(filename);
	else
		return loadSTLb(filename);
}

void saveSTL(TriMesh* mesh, const String& name)
{
	asl::File file(name, asl::File::WRITE);
	file << String(' ', 80);
	file << mesh->indices.length() / 3;
	StreamBuffer buffer;
	for (int i = 0; i < mesh->indices.length(); i += 3)
	{
		Vec3 a = mesh->vertices[mesh->indices[i]],
			b = mesh->vertices[mesh->indices[i + 1]],
			c = mesh->vertices[mesh->indices[i + 2]];
		Vec3 n = ((c - a) ^ (b - a)).normalized();
		buffer << n.x << n.y << n.z;
		buffer << a.x << a.y << a.z;
		buffer << b.x << b.y << b.z;
		buffer << c.x << c.y << c.z;
		buffer << (short)0;
		if (buffer.length() > 1000)
		{
			file << *buffer;
			buffer.clear();
		}
	}
	file << *buffer;
}

// Still only supports OBJs with normals, and with only triangles

SceneNode* loadOBJ(const asl::String& filename)
{
	TextFile file(filename, File::READ);
	if (!file)
		return NULL;

	TriMesh* mesh = new TriMesh();

	Dic<TriMesh*> meshes;
	Dic<Material*> materials;

	materials[""] = new Material;
	meshes[""] = mesh;

	// one mesh per material
	// all meshes will share vertices, normals and texcoords

	Array<Vec3> vertices;
	Array<Vec3> normals;
	Array<Vec2> texcoords;

	mesh->material = materials[""];

	asl::Array<String> indices;

	while (!file.end())
	{
		String line = file.readLine();
		if (line.startsWith('#'))
			continue;
		Array<String> parts = line.split();
		if (parts.length() == 0)
			continue;
		if (parts[0] == 'v')
		{
			vertices << Vec3(parts[1], parts[2], parts[3]);
		}
		else if (parts[0] == "vn")
		{
			normals << Vec3(parts[1], parts[2], parts[3]);
		}
		else if (parts[0] == "vt")
		{
			texcoords << Vec2(parts[1], 1.0f - (float)parts[2]);
		}
		else if (parts[0] == 'f')
		{
			for (int i = 1; i < parts.length(); i++)
			{
				parts[i].split('/', indices);
				mesh->indices << (int)indices[0] - 1;
				if (indices.length() > 1)
					mesh->texcoordsI << (int)indices[1] - 1;
				if (indices.length() > 2)
					mesh->normalsI << (int)indices[2] - 1;
			}
		}
		else if (parts[0] == "usemtl")
		{
			String& name = parts[1];
			if (!meshes.has(name))
			{
				mesh = new TriMesh;
				mesh->material = materials.get(name, materials[""]);
				meshes[name] = mesh;
			}
			mesh = meshes[name];
		}
		else if (parts[0] == "mtllib")
		{
			TextFile matfile(Path(filename).directory() + "/" + parts[1], File::READ);
			Material* mat = materials[""];
			for (auto line : matfile.lines())
			{
				Array<String> parts = line.split();
				if (parts.length() == 0)
					continue;

				if (parts[0] == "newmtl")
				{
					mat = new Material;
					materials[parts[1]] = mat;
				}
				else if (parts[0] == "Kd")
					mat->diffuse = Vec3(parts[1], parts[2], parts[3]);
				else if (parts[0] == "Ks")
					mat->specular = Vec3(parts[1], parts[2], parts[3]);
				else if (parts[0] == "Ke")
					mat->emissive = Vec3(parts[1], parts[2], parts[3]);
				else if (parts[0] == "Ns")
				{
					mat->shininess = parts[1];
					if (mat->shininess < 0.0001f)
						mat->shininess = 10;
				}
				else if (parts[0] == "d")
					mat->opacity = parts[1];
				else if (parts[0] == "map_Kd")
					mat->textureName = parts[1];
			}
		}
	}

	for (auto mat : materials)
	{
		if (mat.value->textureName)
		{
			mat.value->texture = loadPPM(Path(filename).directory() + "/" + mat.value->textureName);
		}
	}

	SceneNode* node = new SceneNode;
	for (auto& mesh : meshes)
	{
		mesh.value->vertices = vertices;
		mesh.value->normals = normals;
		mesh.value->texcoords = texcoords;
		node->children << mesh.value;
	}

	return node;
}



void savePPM(const asl::Array2<asl::Vec3>& image, const asl::String& filename)
{
	File file(filename, File::WRITE);
	String header;
	header << "P6\n" << image.cols() << " " << image.rows() << "\n" << 255 << "\n";
	file << header;
	Array<byte> data(image.cols() * 3);

	for (int i = 0; i < image.rows(); i++)
	{
		for (int j = 0; j < image.cols(); j++)
		{
			Vec3 value = image(i, j) * 255.0f;
			data[j * 3] = (byte)clamp(value.x, 0.0f, 255.0f);
			data[j * 3 + 1] = (byte)clamp(value.y, 0.0f, 255.0f);
			data[j * 3 + 2] = (byte)clamp(value.z, 0.0f, 255.0f);
		}
		file.write(data.ptr(), data.length());
	}
}

asl::Array2<asl::Vec3> loadPPM(const asl::String& filename)
{
	asl::Array2<asl::Vec3> image;
	File file(filename, File::READ);
	if (!file)
		return image;
	String header;
	int nl = 0;
	bool comment = false;
	do {
		char c = file.read<char>();
		if (c == '\n')
		{
			if (!comment)
				nl++;
			comment = false;
		}
		else if (c == '#')
			comment = true;
		if (!comment)
			header << c;
	} while (nl < 3 && !file.end());

	Array<String> parts = header.split();

	if (parts.length() != 4 || parts[0] != "P6")
		return image;
	image.resize(parts[2], parts[1]);

	Array<byte> data(image.cols() * 3);

	for (int i = 0; i < image.rows(); i++)
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

void saveXYZ(const asl::Array2<asl::Vec3>& points, const asl::String& filename)
{
	TextFile file(filename, File::WRITE);

	for (int i = 0; i < points.rows(); i++)
		for (int j = 0; j < points.cols(); j++)
		{
			Vec3 p = points(i, j);
			if (p.z < -1e-5f)
				file.printf("%f %f %f\n", p.x, p.y, p.z);
		}
}

}
