#ifndef SCENE_H
#define SCENE_H

#include <asl/Vec3.h>
#include <asl/Vec2.h>
#include <asl/Matrix4.h>
#include <asl/Array.h>
#include <asl/String.h>

struct Vertex
{
	asl::Vec3 position;
	asl::Vec3 normal;
	asl::Vec2 uv;

	Vertex() {}
	Vertex(asl::Vec3& p): position(p), normal(0, 0, 1), uv(0, 0) {}
	Vertex(asl::Vec3& p, asl::Vec3& n): position(p), normal(n), uv(0, 0) {}
	Vertex(asl::Vec3& p, asl::Vec3& n, asl::Vec2& t): position(p), normal(n), uv(t) {}
};

struct Material
{
	asl::Vec3 diffuse, specular, emissive;
	float shininess, opacity;
	Material();
};

struct SceneNode
{
	bool visible;
	asl::Matrix4 transform;
	SceneNode();
	virtual ~SceneNode() {}
};

struct Shape : public SceneNode
{
    Material* material;
	virtual ~Shape() {}
};

struct TriMesh : public Shape
{
	asl::Array<asl::Vec3> vertices;
	asl::Array<asl::Vec3> normals;
	asl::Array<asl::Vec2> texcoords;
	asl::Array<int> indices;
	asl::Array<int> normalsI;
	asl::Array<int> texcoordsI;

	TriMesh();
};


#endif
