#pragma once

#include "Scene.h"

namespace minirender
{

asl::Shared<minirender::TriMesh> createCube(float size = 1.0f);

asl::Shared<minirender::TriMesh> createCylinder(float radius, float height, int segments = 32, int heightSegments = 1,
                                                bool caps = true);

asl::Shared<minirender::TriMesh> createSphere(float radius, int latSegments = 16, int longSegments = 32);
}
