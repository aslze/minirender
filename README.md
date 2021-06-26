# minirender #

**A useless software rasterizer for 3D meshes using a z-buffer.**

Only requires the [ASL](https://github.com/aslze/asl) library.

Very simple at the moment. It interpolates normals to produce smooth Blinn-Phong diffuse and specular illumination.
But the only supplied mesh reader is for STL, and there is no normal recomputation, so the triangles will look flat.

## Current features

* Rasterization of triangle meshes, using OpenGL-like modelview and projection matrices
* Blinn-Phong illumination (currently with hardcoded colors and properties)
* Rasterization interpolates vertex positions and normals (can create smooth shading)
* Ability to save images in PPM format (very simple and not needing 3rd party libraries)
* Triangle clipping at the near plane
* Textures
* STL and OBJ loaders (only triangles now)

## Possible future features

* Configurable materials
* Additional loader(s) (maybe PLY)
* A hierarchical scene description with meshes and transforms
* Normals computation and polygon triangularization
