\page simple_demo Simple demo

This example shows how move the camera automatically.

## Compile and run the example

Create a build directory and use `cmake` and `make` to compile the code:

```{.sh}
cd ign-rendering/examples/simple_demo
mkdir build
cd build
cmake ..
make
```
Execute the example:

```{.sh}
./simple_demo
[Msg] Loading plugin [ignition-rendering4-ogre]
Engine 'optix' is not supported
===============================
  TAB - Switch render engines  
  ESC - Exit                   
===============================
```

@image html img/simple_demo.gif

## Code

The function `updateCameras()` is called each time the `DisplayCB` function runs. Using the method `SetLocalPosition` from the `Camera` class we can locate the camera in the world:

\snippet examples/simple_demo/GlutWindow.cc update camera
