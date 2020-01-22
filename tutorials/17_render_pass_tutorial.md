# Render pass

This example shows how to add gaussian mode to the camera

use mouse to select objects in the scene.

# Compile and run the example

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

![](img/render_pass.gif)

# Code

Get the render pass system to check if the render engine has one. Then we just need to create some gaussian noise using the mean and the standard deviation and add this noise to the camera.

```{.cpp}
// get render pass system
CameraPtr camera = std::dynamic_pointer_cast<Camera>(sensor);
RenderPassSystemPtr rpSystem = engine->RenderPassSystem();
if (rpSystem)
{
  // add gaussian noise pass
  RenderPassPtr pass = rpSystem->Create<GaussianNoisePass>();
  GaussianNoisePassPtr noisePass =
      std::dynamic_pointer_cast<GaussianNoisePass>(pass);
  noisePass->SetMean(0.1);
  noisePass->SetStdDev(0.08);
  camera->AddRenderPass(noisePass);
}
