#ifndef MINIRENDER_IO_H
#define MINIRENDER_IO_H

#include "Scene.h"
#include <asl/Array2.h>
#include <asl/Vec3.h>

namespace minirender {

SceneNode* loadMesh(const asl::String& filename);

TriMesh* loadSTL(const asl::String& filename);
void saveSTL(TriMesh* mesh, const asl::String& name);

SceneNode* loadOBJ(const asl::String& filename);

void savePPM(const asl::Array2<asl::Vec3>& image, const asl::String& filename);
asl::Array2<asl::Vec3> loadPPM(const asl::String& filename);

void saveXYZ(const asl::Array2<asl::Vec3>& points, const asl::String& filename);

}

#endif
