#define DLL_EXPORTS

#include "Scene.h"

using namespace asl;


SceneNode::SceneNode()
{
	visible = true;
	transform = Matrix4::identity();
}

TriMesh::TriMesh()
{
	material = NULL;
}

Material::Material():
	diffuse(0.7f, 0.7f, 0.9f),
	specular(0.8f, 0.8f, 0.8f),
	emissive(0, 0, 0),
	shininess(12.0f),
	opacity(1.0f)
{}


