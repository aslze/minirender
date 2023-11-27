#ifndef MINIRENDER_RENDERER_H
#define MINIRENDER_RENDERER_H

#include "Scene.h"
#include <asl/Array2.h>

namespace minirender {

asl::Matrix4 projectionOrtho(float l, float r, float b, float t, float n, float f);
asl::Matrix4 projectionPerspective(float l, float r, float b, float t, float n, float f);
asl::Matrix4 projectionFrustum(float fov, float aspect, float n, float f);
asl::Matrix4 projectionFrustumH(float fov, float aspect, float n, float f);
asl::Matrix4 projectionOrtho(float fov, float aspect, float n, float f);
asl::Matrix4 projectionCV(const asl::Matrix4& K, float w, float h, float n, float f);


class Renderer
{
	asl::Array2<asl::Vec3> _image;
	asl::Array2<float> _depth;
	asl::Array2<asl::Vec3> _points;
	asl::Array2<asl::Vec3> _pnormals;
	asl::Array<asl::Vec3> _vertices;
	asl::Array<asl::Vec3> _normals;
	asl::Matrix4 _view;
	asl::Matrix4 _projection;
	asl::Matrix4 _modelview;
	asl::Matrix4 _normalmat;
	asl::Vec3 _lightdir;
	asl::Vec3 _light;
	asl::Vec3 _bgcolor;
	float _ambient;
	float _znear;
	asl::Shared<Scene>     _scene;
	asl::Shared<Material>  _material;
	asl::Shared<Material>  _defmaterial;
	asl::Array<Renderable> _renderables;
	void clipTriangle(float z, Vertex v[3]);
	bool _lighting;
	bool _texturing;
	bool _lightIsPoint;
public:
	Renderer();
	void setSize(int w, int h);
	float aspect() const { return (float)_image.cols() / _image.rows(); }
	void setScene(asl::Shared<Scene> scene);
	void setProjection(const asl::Matrix4& m) { _projection = m; }
	void setView(const asl::Matrix4& m) { _view = m; }
	void setLight(const asl::Vec3& v, bool point = false) { _light = v; _lightIsPoint = point; }
	void setMaterial(asl::Shared<Material> material) { _material = material; }
	void setLighting(bool on) { _lighting = on; }
	void setTexturing(bool on) { _texturing = on; }
	void setBackground(const asl::Vec3& color) { _bgcolor = color; }
	void clear();
	void render();
	void paintMesh(TriMesh* mesh, const asl::Matrix4& transform = asl::Matrix4::identity());
	void paintTriangle(const Vertex& a, const Vertex& b, const Vertex& c, bool world = true);
	asl::Array2<float>     getDepth() const { return _depth; }
	asl::Array2<asl::Vec3> getImage() const;
	asl::Array2<asl::Vec3> getRangeImage();
	asl::Array2<asl::Vec3> getNormalsImage() const { return _pnormals; }
};

}
#endif
