# minirender #

**A useless software rasterizer for 3D meshes using a z-buffer.**

Only requires the [ASL](https://github.com/aslze/asl) library.

It interpolates normals to produce smooth Blinn-Phong diffuse and specular illumination.

## Current features

* Rasterization of triangle meshes, using OpenGL-like modelview and projection matrices
* Blinn-Phong illumination
* Rasterization interpolates vertex positions, normals and texture coordinates (can create smooth shading)
* Ability to save images in PPM format (very simple and not needing 3rd party libraries)
* Triangle clipping at the near plane
* Textures (PPM only)
* STL (bin or text) and (partial) OBJ/MTL loaders (now only triangles and must have normals)
* Simple hierarchical scene with meshes and transforms
* Only one point light, in camera coordinates

## Possible future features

* Additional loader(s) (maybe PLY)
* Normals computation and polygon triangularization
* Light position in world coordinates
* More lights including spots
