This sample loads and renders a 3D model.

Can output images in PPM format or draw on the console, if it supports RGB color codes (e.g. modern Linux consoles, Windows Terminal, Visual Studio Debug console).

```
render [options] model_path
```

model_path can point to an STL file or an OBJ file (triangulated and with normals).

Options:

* `-n <number>` Number of frames to render (default 1). The model will rotate around Z between frames.
* `-d <number>` Camera distance from origin (default 170)
* `-console!` Will render on the console, if the console supports RGB color codes.
* `-save!` Will save frames to numbered PPM files.
* `-w <number>` (and `-h <number>`) Sets image size for rendering (ignored if console used).


```
render -n 100 -console! model.obj
```
