#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"
#include <asl/Array2.h>

asl::Matrix4 projectionOrtho(float l, float r, float b, float t, float n, float f);
asl::Matrix4 projectionPerspective(float l, float r, float b, float t, float n, float f);
asl::Matrix4 projectionFrustum(float fov, float aspect, float n, float f);

class SWRenderer
{
	asl::Array2<asl::Vec3> _image;
	asl::Array2<float> _depth;
	asl::Matrix4 _view;
	asl::Matrix4 _projection;
	asl::Matrix4 _modelview;
	asl::Matrix4 _normal;
	asl::Vec3 _light;
	asl::Vec3 _color;
	asl::Vec3 _specular;
	asl::Vec3 _ambient;
	float _shininess;
	TriMesh* _object;
public:
	SWRenderer();
	void setSize(int w, int h);
	float aspect() const { return (float)_image.cols() / _image.rows(); }
	void setScene(TriMesh* part);
	void setProjection(const asl::Matrix4& m) { _projection = m; }
	void setView(const asl::Matrix4& m) { _view = m; }
	void setLight(const asl::Vec3& v) { _light = v; }
	void clear();
	void render();
	void paintMesh(TriMesh* mesh, const asl::Matrix4& transform);
	void paintTriangle(const Vertex& a, const Vertex& b, const Vertex& c);
	asl::Array2<asl::Vec3> getImage();
};

void saveImage(const asl::Array2<asl::Vec3>& image, const asl::String& filename);
asl::Array2<asl::Vec3> loadImage(const asl::String& filename);

#endif