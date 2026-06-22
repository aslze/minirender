#include "minirender/Scene.h"

using namespace asl;

namespace minirender
{

SceneNode::SceneNode()
{
	visible = true;
	transform = Matrix4::identity();
}

void SceneNode::collectShapes(Array<Renderable>& list, const Matrix4& xform)
{
	auto tr = xform * transform;
	for (auto& node : children)
	{
		node->collectShapes(list, tr);
	}
}

BBox SceneNode::getBbox(const Matrix4& xform, bool includeChildren, bool local) const
{
	BBox box;
	for (auto& node : children)
		box += node->getBbox(xform * transform);
	return box;
}

void TriMesh::collectShapes(Array<Renderable>& list, const Matrix4& xform)
{
	auto tr = xform * transform;
	if (indices.length() > 0)
		list << Renderable(this, tr);
	for (auto& node : children)
		node->collectShapes(list, tr);
}

BBox TriMesh::getBbox(const Matrix4& xform, bool includeChildren, bool local) const
{
	BBox box;
	Matrix4 transform2 = xform * transform;
	if (local)
		for (auto& i : indices)
			box += vertices[i];
	else
		for (auto& p : vertices)
			box += transform2 * p;
	if (includeChildren)
	{
		for (auto& node : children)
			box += node->getBbox(transform2, includeChildren);
	}
	return box;
}

TriMesh::TriMesh()
{
	material = NULL;
}

void TriMesh::applyTransform()
{
	for (auto& p : vertices)
		p = transform * p;

	Matrix4 invTrans = transform.inverse().transposed();
	for (auto& n : normals)
		n = (invTrans % n).normalized();

	transform = Matrix4::identity();
}

Material::Material() :
    diffuse(0.7f, 0.7f, 0.9f), specular(0.8f, 0.8f, 0.8f), emissive(0, 0, 0), shininess(12.0f), opacity(1.0f)
{
}

Scene::Scene()
{
	ambientLight = 0.1f;
}

Shared<TriMesh> merge(const Array<Shared<Shape>>& shapes)
{
	Shared<TriMesh> result = new TriMesh;
	for (auto& shape : shapes)
	{
		if (auto mesh = shape.as<TriMesh>())
		{
			result->material = mesh->material;
			int v0 = result->vertices.length();
			int n0 = result->normals.length();
			int t0 = result->texcoords.length();
			result->vertices.reserve(v0 + mesh->vertices.length());
			result->normals.reserve(n0 + mesh->normals.length());
			result->texcoords.reserve(t0 + mesh->texcoords.length());
			result->indices.reserve(result->indices.length() + mesh->indices.length());
			auto m = mesh->transform;
			for (auto& v : mesh->vertices)
				result->vertices << m * v;
			for (auto& n : mesh->normals)
				result->normals << (m % n).normalized();
			for (auto& t : mesh->texcoords)
				result->texcoords << t;
			for (auto& i : mesh->indices)
				result->indices << i + v0;
			for (auto& i : mesh->normalsI)
				result->normalsI << i + n0;
			for (auto& i : mesh->texcoordsI)
				result->texcoordsI << i + t0;
		}
	}

	return result;
}

}
