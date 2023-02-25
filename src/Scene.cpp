#include "minirender/Scene.h"

using namespace asl;

namespace minirender {

SceneNode::SceneNode()
{
	visible = true;
	transform = Matrix4::identity();
}

void SceneNode::collectShapes(Array<Renderable>& list, const asl::Matrix4& xform)
{
	auto tr = xform * transform;
	for (auto& node : children)
	{
		node->collectShapes(list, tr);
	}
}

BBox SceneNode::getBbox(const asl::Matrix4& xform) const
{
	BBox box;
	for (auto& node : children)
		box += node->getBbox(xform * transform);
	return box;
}

void TriMesh::collectShapes(asl::Array<Renderable>& list, const asl::Matrix4& xform)
{
	auto tr = xform * transform;
	list << Renderable(this, tr);
	for (auto& node : children)
		node->collectShapes(list, tr);
}

BBox TriMesh::getBbox(const asl::Matrix4& xform) const
{
	BBox box;
	for (auto& p : vertices)
		box += xform * transform * p;
	for (auto& node : children)
		box += node->getBbox(xform * transform);
	return box;
}

TriMesh::TriMesh()
{
	material = NULL;
}

Material::Material() :
	diffuse(0.7f, 0.7f, 0.9f),
	specular(0.8f, 0.8f, 0.8f),
	emissive(0, 0, 0),
	shininess(12.0f),
	opacity(1.0f)
{}

Scene::Scene()
{
	ambientLight = 0.1f;
}

}
