\page actor_animation Actor animation tutorial

This tutorial will show you how to use the Ignition Rendering library to create an actor animation.

## Code

In this section we will describe the main classes and methods used to create the actor. The basic steps are:

 - Create a visual for the actor
 - Attach mesh
 - Get skeleton
 - The skeleton will be animated by `GlutWindow`

Create a `Visual` pointer with the scene manager and set the position and rotation of the object:

\snippet examples/actor_animation/Main.cc create a visual for the actor

Create a `MeshDescriptor` class with the mesh name of the actor, then using the `MeshManager` Singleton class, load the mesh in the descriptor:

\snippet examples/actor_animation/Main.cc create mesh

Finally, attach the mesh to the visual:

\snippet examples/actor_animation/Main.cc added mesh

We can check if the mesh has a skeleton. This will allows us to check if we can animate the actor in the scene:

\snippet examples/actor_animation/Main.cc check skeleton

There are two main function in the `GlutWindow.c` file:

 - **void initAnimation()**: This function will check if the skeleton has an animation. Then it will load the animation using a `bvh` file (an animation file format).

\snippet examples/actor_animation/GlutWindow.cc init animation

And finally, the animation will be initialized:

\snippet examples/actor_animation/GlutWindow.cc index animation

 - **void updatePose()**: This function will update the skeleton over the time to create the animation of the actor.
\snippet examples/actor_animation/GlutWindow.cc update pose


## Compile and run the example

Create a build directory and use `cmake` and `make` to compile the code:

```{.sh}
cd ign-rendering/examples/actor_animation
mkdir build
cd build
cmake ..
make
```

When the code is compiled you can execute the example with the following command. Using the left and right button of the mouse you can move around the scene and even zoom in and out.

```{.sh}
./actor_animation
```

@image html img/actor_animation.png
