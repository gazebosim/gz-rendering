\page render_pass Render pass

This example shows how to add gaussian mode to the camera

## Compile and run the example

Create a build directory and use `cmake` and `make` to compile the code:

```{.sh}
cd ign-rendering/examples/render_pass
mkdir build
cd build
cmake ..
make
```
Excute the example:

```{.sh}
./render_pass
[Msg] Loading plugin [ignition-rendering4-ogre]
Engine 'optix' is not supported
===============================
  TAB - Switch render engines  
  ESC - Exit                   
===============================
```

@image html img/render_pass.gif

## Code

Get the render pass system to check if the render engine has one. Then we just need to create some gaussian noise using the mean and the standard deviation and add this noise to the camera.

\snippet examples/render_pass/Main.cc get render pass system
