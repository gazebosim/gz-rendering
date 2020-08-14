\page particles Particles

This example shows how to include a particle emitter in the scene.

## Compile and run the example

Create a build directory and use `cmake` and `make` to compile the code:

```{.sh}
cd ign-rendering/examples/particles_demo
mkdir build
cd build
cmake ..
make
```
Execute the example:

```{.sh}
./particles_demo
[Msg] Loading plugin [ignition-rendering4-ogre]
Engine 'optix' is not supported
===============================
  TAB - Switch render engines
  ESC - Exit
===============================
```
@image html img/particles.png

## Code

A particle emitter generates a stream of particles. We can create a particle
emitter in the same way we create other objects in the scene. We just need to
use the scene to create the emitter using the method `CreateParticleEmitter`. It's possible to configure a few attributes of the emitter and its particles:

* Type: `EM_POINT` emitters emit particles from a single point. `EM_BOX`,
`EM_CYLINDER` and `EM_ELLIPSOID` emitters emit randomly from a box, cylinder or
ellipsoid respectively.

* Emitter size: Adjust the dimensions of the volume from which the particles are
emitted.

* Rate: Set the amount of particles per second emitted.

* Duration: This parameter can be used to automatically disable the emitter
after some time.

* Emitting: Enable/Disable the emission of particles.

* Particle size: Adjust the particle dimensions.

* Lifetime: Set the amount of seconds each particles "lives" before being
destroyed.

* Material: Set the material which all particles in the emitter will use.

* Velocity: Set a velocity range and each particle is emitted with a random
velocity within this range.

* Color range: Set the color for all particles emitted. You have two options to
set the color.
  * Using two color values: The actual color is interpolated between these two
  colors along the lifetime of the particle.
  * Using an image: The colors are taken from a specified image file. The range
  of color values begins from the left side of the image and move to the right
  over the lifetime of the particle, therefore only the horizontal dimension of
  the image is used.

* Scale rate: Adjust the amount by which to scale the particles in both `X` and
`Y` direction per second.

Here's the snippet of code from `examples/particles_demo/Main.cc` that includes
a particle emitter to render particles to look like smoke:

```
// create particle material
MaterialPtr particleMaterial = _scene->CreateMaterial();
particleMaterial->SetDiffuse(0.7, 0.7, 0.7);
particleMaterial->SetTexture(RESOURCE_PATH + "/smoke.png");
particleMaterial->SetAlphaFromTexture(true);
particleMaterial->SetDepthWriteEnabled(false);

// create particle emitter.
ParticleEmitterPtr emitter = _scene->CreateParticleEmitter();
emitter->SetLocalPose({0, 0, 0, 0, -1.57, 0});
emitter->SetRate(10);
emitter->SetParticleSize({1, 1, 1});
emitter->SetLifetime(2);
emitter->SetVelocityRange(10, 20);
emitter->SetMaterial(particleMaterial);
emitter->SetColorRangeImage(RESOURCE_PATH + "/smokecolors.png");
emitter->SetScaleRate(10);
emitter->SetEmitting(true);
root->AddChild(emitter);
```