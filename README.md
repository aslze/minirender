# minirender #

**A useless software rasterizer for 3D meshes using a z-buffer.**

Only requires the [ASL](https://github.com/aslze/asl) library.

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
* STL (bin or text) and (partial) OBJ/MTL loaders (now only triangles and must have normals)
* Simple hierarchical scene with meshes and transforms
* Only one directional light, in camera space

## Possible future features

* Additional loader(s) (maybe PLY)
* Normals computation and polygon triangularization
* Light position in world coordinates
* More lights including spots


## Console output

The included [sample](samples/README.md) can render on the console in real time. The `-console` option enables this, and `-t` indicates 10 seconds of animation.

```
render -console! -t 10 scene.obj
```

![Console output](https://github.com/aslze/minirender/releases/download/0.1.3/output-console.png)
