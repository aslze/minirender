#include <asl/TextFile.h>
#include <asl/StreamBuffer.h>
#include "Scene.h"

using namespace asl;

TriMesh* loadSTLa(const asl::String& filename)
{
	TextFile file(filename, File::READ);
	if(!file)
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

	TriMesh* obj = new TriMesh();
	int nf = file.read<int>();

	Array<byte> data(3 * sizeof(float) + 3 * 3 * sizeof(float) + 2);

	obj->normals.reserve(nf);
	obj->vertices.reserve(nf*3);
	obj->indices.reserve(nf*3);
	obj->normalsI.reserve(nf*3);

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

TriMesh* loadSTL(const asl::String& filename)
{
	Array<byte> bytes = asl::File(filename).firstBytes(5);
	if(bytes.length() < 5)
		return NULL;

	if(String((char*)bytes.ptr(), 5) == "solid")
		return loadSTLa(filename);
	else
		return loadSTLb(filename);
}

void saveSTL(TriMesh* mesh, const String& name)
{
	asl::File file(name, asl::File::WRITE);
	file << String(' ', 80);
	file << mesh->indices.length()/3;
	for(int i=0; i < mesh->indices.length(); i+=3)
	{
		Vec3 a = mesh->vertices[mesh->indices[i]],
		b = mesh->vertices[mesh->indices[i+1]],
		c = mesh->vertices[mesh->indices[i+2]];
		Vec3 n = ((c-a) ^ (b-a)).normalized();
		file << n.x << n.y << n.z;
		file << a.x << a.y << a.z;
		file << b.x << b.y << b.z;
		file << c.x << c.y << c.z;
		file << (short)0;
	}
}
