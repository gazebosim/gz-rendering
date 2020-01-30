\page custom_shaders Custom shaders

This example will create two images (RGB and Depth) based on the scene. The scene will allow us to introduce shaders from a file.

## Compile and run the example

Create a build directory and use `cmake` and `make` to compile the code:

```{.sh}
cd ign-rendering/examples/camera_tracking
mkdir build
cd build
cmake ..
make
```

```
./camera_tracking
Image saved: depth.png
Image saved: regular.png
```

@image html img/custom_shaders_rgb.png
@image html img/custom_shaders_depth.png

## Code

In the first lines of the `custom_shaders.cc` file there are some constants defined with the name of the shaders that the scene will use:

\snippet examples/custom_shaders/custom_shaders.cc init shaders variables

We can use the environment variable `IGN_RENDERING_RESOURCE_PATH` to define where are these files located. The code will add to this path some directories (/ogre/media/materials/programs/):

\snippet examples/custom_shaders/custom_shaders.cc Get shader parameters path

Then the shaders will be applied on the object's material:

\snippet examples/custom_shaders/custom_shaders.cc add shader to material
