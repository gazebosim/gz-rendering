\page render_order Render order

This example shows how to set the render order for coplanar poligons.

The material API allows to change the render order. When polygons are coplanar, you can get problems
with `depth fighting` where the pixels from the two polys compete for the same screen pixel. As you
can see in the following image:

@image html img/render_order_bad.png

The method `SetRenderOrder` in the Material class allows to avoid this issues. The higher value will
be rendered on top of the other coplanar polygons.

In the simple_demo example you can find two materials with different render order. Red material (SetRenderOrder(3))
has a higher value than the white material (SetRenderOrder(3)).

\snippet examples/render_pass/Main.cc red material

\snippet examples/simple_demo/Main.cc white material

As you can see in the following image the z-fighting issue is resolved.

@image html img/render_order_good.png

You can set this in your SDF file including in the material tag a new tag called `render_order` with
a float value:

```xml
<material>
  <render_order>5</render_order>
  <ambient>0 0 1 1</ambient>
  <diffuse>0 0 1 1</diffuse>
  <specular>0 0 1 1</specular>
</material>
```

Clone the source code, create a build directory and use `cmake` and `make` to compile the code:

```{.sh}
git clone https://github.com/ignitionrobotics/ign-rendering
cd ign-rendering/examples/simple_demo
mkdir build
cd build
cmake ..
make
```

Execute the example:

```{.sh}
./simple_demo
```
