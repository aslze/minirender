# minirender #

**A useless software rasterizer for 3D meshes using a z-buffer.**

Only requires the [ASL](https://github.com/aslze/asl) 1.11.7+ library (automatically downloaded and built if needed).

It interpolates normals to produce smooth Blinn-Phong diffuse and specular illumination.

![Sample output](https://github.com/aslze/minirender/releases/download/0.1.3/output.jpg)

(image rendered from the model attached to release 0.1.3 in around 30 ms.)


## Current features

* Rasterization of triangle meshes, using OpenGL-like modelview and projection matrices
* Blinn-Phong illumination
* Rasterization interpolates vertex positions, normals and texture coordinates (can create smooth shading)
* Ability to save images in PPM format (very simple and not needing 3rd party libraries)
* Triangle clipping at the near plane
* Textures (PPM only)
* Loaders for:
  - STL (binary or text)
  - OBJ/MTL
  - X3D (`IndexedFaceSet` and `IndexedTriangleSet` meshes with scene hierarchy and materials)
* Simple hierarchical scene with meshes and transforms
* Only one point light in world-coordinates

## Possible future features

* Additional loader(s) (maybe PLY)
* Normals computation with autosmoothing
* More lights including spots


## Console output

The included [sample](samples/README.md) can render on the console in real time. The `-console` option enables this, and `-t` indicates 10 seconds of animation.

```
render -console! -t 10 scene.obj
```

![Console output](https://github.com/aslze/minirender/releases/download/0.1.3/output-console.png)
