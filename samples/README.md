This sample loads and renders a 3D model.

Can output images in PPM format or draw on the console, if it supports RGB color codes (e.g. modern Linux consoles, Windows Terminal, Visual Studio Debug console).

```
render [options] model_path
```

model_path can point to an STL file or an OBJ file (triangulated and with normals).

Options:

* `-n <number>` Number of frames to render (default 1). The model will rotate by 'rx', 'rz' between frames.
* `-t <number>` Total time in seconds, if given, and will produce multiple frames
* `-d <number>` Camera distance from origin (default: automatic to fit scene in view)
* `-console!` Will render on the console, if the console supports RGB color codes.
* `-save!` Will save frames to numbered PPM files.
* `-o <string>` Output file name (use `--` for stdout, or `...%04i...` for multiple frames)
* `-w <number>` (and `-h <number>`) Sets image size for rendering (ignored if console used).
* `-yaw <number>` Yaw angle (rotation around Z) of camera in degrees
* `-tilt <number>` Tilt angle (rotation around X) of camera in degrees
* `-bgcolor <r,g,b>` Set background color (default black)
* `-rx <number>` and `-rz <number>` Rotation around X and Z in deg/s (default RZ 40, RX 0)
* `-oldconsole!` The console only supports 256 colors

Render 10 second animation in real time on the console:

```
render -console! -t 10 scene.obj
```

Older consoles (e.g. Gnome on Ubuntu 14) do not support 24 bit RGB colors, but they can use a palette of 256 colors. Add the option `-oldconsole` together with `-console` to make the rendering use this palette (worse quality). Console rendering has been tested on Ubuntu 10 (256 colors), Ubuntu 18, Windows Terminal and Windows 10 console (cmd).

Render 50 frames, rotating 30 deg/s, and save them as PPM files.

```
render -o images-%04i.ppm -rz 30 -n 50 scene.obj
```

There is a sample file in the assets of release 0.1.3 you can use ("sample_model.zip").

