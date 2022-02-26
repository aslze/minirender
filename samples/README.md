This sample loads and renders a 3D model.

Can output images in PPM format or draw on the console, if it supports RGB color codes (e.g. modern Linux consoles, Windows Terminal, Visual Studio Debug console).

```
render [options] model_path
```

model_path can point to an STL file or an OBJ file (triangulated and with normals).

Options:

* `-n <number>` Number of frames to render (default 1). The model will rotate around Z between frames.
* `-t <number>` Total time, if given (seconds)
* `-d <number>` Camera distance from origin (default 140)
* `-console!` Will render on the console, if the console supports RGB color codes.
* `-save!` Will save frames to numbered PPM files.
* `-w <number>` (and `-h <number>`) Sets image size for rendering (ignored if console used).
* `-rx <number>` and `-rz <number>` Rotation around X and Z in deg/s (default RZ 40, RX 0)

Render 10 second animation in real time on the console:

```
render -console! -t 10 scene.obj
```

Render 50 frames, rotating 30 deg/s, and save them as PPM files.

```
render -save! -rz 30 -n 50 scene.obj
```

There is a sample file in the latest release ("sample_model.zip").
