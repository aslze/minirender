#include <asl/TextFile.h>
#include <asl/Map.h>
#include <asl/Path.h>
#include <asl/Xml.h>
#include "minirender/io.h"

using namespace asl;

namespace minirender
{

inline Vec3 toVec3(const String& s)
{
	auto a = s.split_<float>().resize(3);
	return Vec3(a[0], a[1], a[2]);
}

Array<int> triangulateIndices(const Array<int>& indices)
{
	Array<int> tris;
	tris.reserve(indices.length());
	int i = 0, j = 0;
	while (i < indices.length())
	{
		for (j = i + 1; j < indices.length() - 1; j++)
		{
			if (indices[j] == -1 || indices[j + 1] == -1)
				break;
			tris << indices[i] << indices[j] << indices[j + 1];
		}
		i = j + 2;
	}
	return tris;
}

SceneNode* getSceneItem(Xml& e)
{
	SceneNode* node;
	if (e.tag() == "Transform" || e.tag() == "Group")
	{
		auto rotation = (e["rotation"] | "0 0 1 0").split();
		auto translation = (e["translation"] | "0 0 0").split();
		auto scale = (e["scale"] | "1 1 1").split();
		node = new SceneNode;
		node->transform = Matrix4::translate(translation[0], translation[1], translation[2]) *
		                  Matrix4::rotate(Vec3(rotation[0], rotation[1], rotation[2]), rotation[3]) *
		                  Matrix4::scale(Vec3(scale[0], scale[1], scale[2]));

		for (auto& child : e.children())
		{
			auto n = getSceneItem(child);
			if (n)
				node->children << n;
		}

		return node;
	}
	else if (e.tag() == "Shape")
	{
		TriMesh* mesh = new TriMesh;
		auto     appx = e("Appearance");

		mesh->material = new Material;

		if (Xml mat = appx("Material"))
		{
			mesh->material->diffuse = toVec3(mat["diffuseColor"]);
			mesh->material->specular = toVec3(mat["specularColor"]);
			mesh->material->emissive = toVec3(mat["emissiveColor"]);
			mesh->material->shininess = float(mat["shininess"] | "0.5") * 10;
		}

		if (Xml tex = appx("ImageTexture"))
		{
			//
		}

		Xml ifs = e("IndexedFaceSet");
		Xml its = e("IndexedTriangleSet");

		if (Xml g = ifs ? ifs : its)
		{
			Array<float> verts = g("Coordinate")["point"].split_<float>(); // TODO remove ',' !
			Array<float> normals = g("Normal")["vector"].split_<float>();
			Array<float> uvs = g("TextureCoordinate")["point"].split_<float>();

			mesh->vertices.reserve(verts.length() / 3);
			mesh->normals.reserve(normals.length() / 3);
			mesh->texcoords.reserve(uvs.length() / 2);

			for (int i = 0; i < verts.length(); i += 3)
				mesh->vertices << Vec3(verts[i], verts[i + 1], verts[i + 2]);

			for (int i = 0; i < normals.length(); i += 3)
				mesh->normals << Vec3(normals[i], normals[i + 1], normals[i + 2]);

			for (int i = 0; i < uvs.length(); i += 2)
				mesh->texcoords << Vec2(uvs[i], uvs[i + 1]);

			if (ifs)
			{
				mesh->indices = g["coordIndex"].split_<int>();
				mesh->texcoordsI = g["texCoordIndex"].split();
				mesh->normalsI = g["normalIndex"].split();

				mesh->indices = triangulateIndices(mesh->indices);
				mesh->texcoordsI = !mesh->texcoordsI ? mesh->indices.clone() : triangulateIndices(mesh->texcoordsI);
				mesh->normalsI = !mesh->normalsI ? mesh->indices.clone() : triangulateIndices(mesh->normalsI);
			}
			else
			{
				mesh->indices = g["index"].split_<int>();
				mesh->texcoordsI = mesh->indices.clone();
				mesh->normalsI = mesh->indices.clone();
			}

			if (!mesh->normals)
			{
				mesh->normalsI.clear();
				for (int i = 0, j = 0; i < mesh->indices.length(); i += 3, j++)
				{
					Vec3 a = mesh->vertices[mesh->indices[i]];
					Vec3 b = mesh->vertices[mesh->indices[i+1]];
					Vec3 c = mesh->vertices[mesh->indices[i+2]];
					Vec3 n = ((b - a) ^ (c - a)).normalized();
					mesh->normals << n;
					mesh->normalsI << j << j << j;
				}
			}

			if (!mesh->texcoords)
			{
				mesh->texcoords << Vec2(0, 0);
				mesh->texcoordsI = Array<int>(mesh->indices.length(), 0);
			}
		}

		return mesh;
	}

	return NULL;
}

SceneNode* loadX3D(const asl::String& filename)
{
	Xml x3d = Xml::read(filename);

	if (!x3d || x3d.tag() != "X3D")
		return NULL;

	Xml scene = x3d("Scene");

	if (!scene)
		return NULL;

	SceneNode* root = new SceneNode();
	
	/*
	Dic<TriMesh*>  meshes;
	Dic<Material*> materials;
	materials[""] = new Material;
	meshes[""] = mesh;
	*/

	// first pass: look for DEF/USE: replace USE with copies?
	// or store meshes/materials in a map and reuse them?

	for (auto& e : scene.children())
	{
		auto node = getSceneItem(e);

		if (node)
			root->children << node;
	}

	return root;
}

}
