#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"
#include <asl/Array2.h>

asl::Matrix4 projectionOrtho(float l, float r, float b, float t, float n, float f);
asl::Matrix4 projectionPerspective(float l, float r, float b, float t, float n, float f);
asl::Matrix4 projectionFrustum(float fov, float aspect, float n, float f);
asl::Matrix4 projectionFrustumH(float fov, float aspect, float n, float f);
asl::Matrix4 projectionOrtho(float fov, float aspect, float n, float f);

class SWRenderer
{
	asl::Array2<asl::Vec3> _image;
	asl::Array2<float> _depth;
	asl::Array2<asl::Vec3> _points;
	asl::Array2<asl::Vec3> _texture;
	asl::Matrix4 _view;
	asl::Matrix4 _projection;
	asl::Matrix4 _modelview;
	asl::Matrix4 _normalmat;
	asl::Vec3 _lightdir;
	asl::Vec3 _color;
	asl::Vec3 _specular;
	float _ambient;
	float _shininess;
	Scene* _scene;
	void clipTriangle(float z, Vertex v[3]);
public:
	SWRenderer();
	void setSize(int w, int h);
	float aspect() const { return (float)_image.cols() / _image.rows(); }
	void setScene(Scene* scene);
	void setProjection(const asl::Matrix4& m) { _projection = m; }
	void setView(const asl::Matrix4& m) { _view = m; }
	void setLight(const asl::Vec3& v) { _lightdir = v; }
	void setTexture(const asl::Array2<asl::Vec3>& image) { _texture = image; }
	void clear();
	void render();
	void paintMesh(TriMesh* mesh, const asl::Matrix4& transform = asl::Matrix4::identity());
	void paintTriangle(const Vertex& a, const Vertex& b, const Vertex& c, bool world = true);
	asl::Array2<asl::Vec3> getImage();
	asl::Array2<asl::Vec3> getRangeImage() { return _points; }
};

#endif
