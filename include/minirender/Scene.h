#ifndef SCENE_H
#define SCENE_H

#include <asl/Vec3.h>
#include <asl/Vec2.h>
#include <asl/Matrix4.h>
#include <asl/Array.h>
#include <asl/Array2.h>
#include <asl/String.h>
#include <asl/Pointer.h>

namespace minirender {

struct Vertex
{
	asl::Vec3 position;
	asl::Vec3 normal;
	asl::Vec2 uv;

	Vertex() {}
	Vertex(const asl::Vec3& p) : position(p), normal(0, 0, 1), uv(0, 0) {}
	Vertex(const asl::Vec3& p, const asl::Vec3& n) : position(p), normal(n), uv(0, 0) {}
	Vertex(const asl::Vec3& p, const asl::Vec3& n, const asl::Vec2& t) : position(p), normal(n), uv(t) {}
};

struct BBox
{
	asl::Vec3 pmin, pmax;
	BBox() { pmin = asl::Vec3(1, 1, 1) * asl::infinity(); pmax = -pmin; }
	BBox& operator+=(const BBox& box) { pmin = min(pmin, box.pmin); pmax = max(pmax, box.pmax); return *this; }
	BBox& operator+=(const asl::Vec3& p) { pmin = min(pmin, p); pmax = max(pmax, p); return *this; }
	asl::Vec3 size() const { return max(pmax - pmin, asl::Vec3::zeros()); }
	asl::Vec3 center() const { return (pmax + pmin) / 2; }
};

struct TriMesh;

struct Material
{
	asl::Vec3 diffuse, specular, emissive;
	float shininess, opacity;
	asl::Array2<asl::Vec3> texture;
	asl::String textureName;
	Material();
};

struct Renderable
{
	TriMesh* mesh;
	asl::Matrix4 transform;
	Renderable(): mesh(0) {}
	Renderable(TriMesh* mesh, const asl::Matrix4& transform) : mesh(mesh), transform(transform) {}
};

struct SceneNode
{
	bool visible;
	asl::Matrix4 transform;
	asl::Array<asl::Shared<SceneNode>> children;
	SceneNode();
	virtual ~SceneNode() {}
	virtual void collectShapes(asl::Array<Renderable>& list, const asl::Matrix4& xform);
	virtual BBox getBbox(const asl::Matrix4& xform = asl::Matrix4::identity()) const;
};

struct Shape : public SceneNode
{
	asl::Shared<Material> material;
	virtual ~Shape() {}
	virtual void applyTransform() {}
};

struct TriMesh : public Shape
{
	asl::Array<asl::Vec3> vertices;
	asl::Array<asl::Vec3> normals;
	asl::Array<asl::Vec2> texcoords;
	asl::Array<int> indices;
	asl::Array<int> normalsI;
	asl::Array<int> texcoordsI;

	virtual void collectShapes(asl::Array<Renderable>& list, const asl::Matrix4& xform);
	virtual BBox getBbox(const asl::Matrix4& xform = asl::Matrix4::identity()) const;
	virtual void applyTransform();

	TriMesh();
};

struct Scene : public SceneNode
{
	float ambientLight;
	asl::Vec3 light;
	Scene();
	void add(const asl::Shared<SceneNode>& node) { children << node; }
};

}

#endif
