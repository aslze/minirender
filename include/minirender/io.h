#ifndef MINIRENDER_IO_H
#define MINIRENDER_IO_H

#include "Scene.h"
#include <asl/Array2.h>
#include <asl/Vec3.h>

namespace minirender
{
asl::Shared<SceneNode> loadMesh(const asl::String& filename);

asl::Shared<TriMesh> loadSTL(const asl::String& filename);

void saveSTL(asl::Shared<TriMesh> mesh, const asl::String& name);

asl::Shared<SceneNode> loadOBJ(const asl::String& filename);

void savePPM(const asl::Array2<asl::Vec3>& image, const asl::String& filename);

asl::Array2<asl::Vec3> loadPPM(const asl::String& filename);

void saveXYZ(const asl::Array2<asl::Vec3>& points, const asl::String& filename, const asl::Matrix4& m = asl::Matrix4::identity());

}

#endif
