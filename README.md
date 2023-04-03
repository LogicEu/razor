# razor

> Simple 3D software rasterizer written in C on top of my 
> [simple pixel engine](https://github.com/LogicEu/spxe.git). Some of the features 
> it supports are:

* Dynamically Loading 3D models (.obj)
* Dynamically Loading Textures (.png, .jpeg, .gif, .ppm)
* Procedurally Generated Terrain
* Texture Mapping
* Basic Shading
* Z Buffer
* Font Rendering

> It is meant to be a basic, simple and easy example of how to render 3D triangles
> in a 2D screen through the scanline and Z buffer algorithms in combination with
> some 3D math.

![alt text](https://github.com/LogicEu/razor/blob/main/assets/images/image.png?raw=true)

## Controls

> Point the camera in 3D space using the mouse to direct your view.

| Key | Action |
| --- | --- |
| W | Move camera forwards |
| S | Move camera backwards |
| D | Move camera to the right |
| A | Move camera to the left |
| Z | Move camera up |
| X | Move camera down |
| Space | Switch between full triangle rasterization or wireframe rendering |
| Escape | Quit |

## Submodules

* [utopia](https://github.com/LogicEu/utopia.git) Collection data structures in C
* [fract](https://github.com/LogicEu/fract.git) 2D and 3D math for graphics and games
* [mass](https://github.com/LogicEu/mass.git) 3D model loader library
* [imgtool](https://github.com/LogicEu/imgtool.git) Save and load images easily
* [spxe](https://github.com/LogicEu/spxe.git) Simple pixel engine and renderer

## Third Party Dependencies

> The main dependencies are common image format libraries needed by
> [imgtool](https://github.com/LogicEu/imgtool.git).

* [libjpeg](https://github.com/thorfdbg/libjpeg.git)
* [libpng](https://github.com/glennrp/libpng.git)
* [libz](https://github.com/madler/zlib.git)

> The windowing system depends on OpenGL libraries aswell:

* [GLFW](https://github.com/glfw/glfw.git)
* [GLEW](https://github.com/nigels-com/glew.git) (only on Linux and Windows)

## Try it

> If you have all third party dependencies installed on your system you can
> easily try razor with the following commands:

```shell
git clone --recursive https://github.com/LogicEu/razor.git
cd razor
make -j # or ./build.sh all
./razor
```

## Compile

> Both build script currently work on Linux an MacOS. The compiled binary will
> be created at the root of the repository. 

```shell
make -j # or ./build.sh all
```
> Make sure to clean after compiling.

```shell
./build.sh clean # or make clean
```

