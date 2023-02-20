#include <asl/TextFile.h>
#include <asl/Map.h>
#include <asl/Path.h>
#include <asl/Xml.h>
#include "minirender/io.h"

using namespace asl;

namespace minirender
{
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
	}
	else if (e.tag() == "Shape")
	{
		TriMesh* mesh = new TriMesh;
		auto     appx = e("Appearance");

		if (Xml mat = appx("Material"))
		{
			//
		}

		if (Xml tex = appx("ImageTexture"))
		{
			//
		}

		if (Xml g = e("IndexedFaceSet")) // unify with IndexedTriangleSet
		{
			Array<float> verts = g("Coordinate")["point"].split();
			Array<float> normals = g("Normal")["vector"].split();
			Array<float> uvs = g("TextureCoordinate")["point"].split();

			for (int i = 0; i < verts.length(); i += 3)
			{
				mesh->vertices << Vec3(verts[i], verts[i + 1], verts[i + 2]);
			}
			for (int i = 0; i < normals.length(); i += 3)
			{
				mesh->normals << Vec3(normals[i], normals[i + 1], normals[i + 2]);
			}

			for (int i = 0; i < uvs.length(); i += 2)
			{
				mesh->texcoords << Vec2(uvs[i], uvs[i + 1]);
			}

			mesh->indices = e["coordIndex"].split();
			mesh->texcoordsI = e["texCoordIndex"].split();
			mesh->normalsI = e["normalIndex"].split();

			// IndexedTriangleSet only has "index", same for all

			// rearrange indices, texcoordsI, normalsI to form triangles
			// a b c d -1 -> a b c c d a

			if (!mesh->texcoordsI)
				mesh->texcoordsI = mesh->indices.clone();

			if (!mesh->normalsI)
				mesh->normalsI = mesh->indices.clone();

			if (!mesh->normals)
			{
				mesh->normals << Vec3(1, 0, 0);
				mesh->normalsI.clear();
				for (int i = 0; i < mesh->indices.length(); i += 4)
				{
					mesh->normalsI << 0 << 0 << 0 << -1;
				}
			}

			if (!mesh->texcoords)
			{
				mesh->texcoords << Vec2(0, 0);
				mesh->texcoordsI.clear();
				for (int i = 0; i < mesh->indices.length(); i += 4)
				{
					mesh->texcoordsI << 0 << 0 << 0 << -1;
				}
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

	TriMesh* mesh = new TriMesh();

	Dic<TriMesh*>  meshes;
	Dic<Material*> materials;

	materials[""] = new Material;
	meshes[""] = mesh;

	// one mesh per material
	// all meshes will share vertices, normals and texcoords

	Array<Vec3> vertices;
	Array<Vec3> normals;
	Array<Vec2> texcoords;

	for (auto& e : scene.children())
	{
		auto node = getSceneItem(e);

		// add to children
	}

	mesh->material = materials[""];

	return mesh;
}

}
