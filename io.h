#ifndef MINIRENDERIO_H
#define MINIRENDERIO_H

#include "Scene.h"
#include <asl/Array2.h>
#include <asl/Vec3.h>

TriMesh* loadSTL(const asl::String& filename);
void saveSTL(TriMesh* mesh, const asl::String& name);

TriMesh* loadOBJ(const asl::String& filename);

void savePPM(const asl::Array2<asl::Vec3>& image, const asl::String& filename);
asl::Array2<asl::Vec3> loadPPM(const asl::String& filename);

void saveXYZ(const asl::Array2<asl::Vec3>&points, const asl::String& filename);

#endif
