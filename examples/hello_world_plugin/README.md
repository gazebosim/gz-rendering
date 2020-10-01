# Engine plugin

This example shows how to create a plugin that integrates a rendering engine with
Ignition Rendering.

## Build

From the root of the `ign-rendering` repository, do the following to build the example:

~~~
cd examples/hello_world_plugin
mkdir build
cd build
cmake ..
make
~~~

This will generate the `HelloWorldPlugin` library under `build`.
The exact name of the library file depends on the operating system
such as `libHelloWorldPlugin.so` on Linux, `libHelloWorldPlugin.dylib` on macOS,
and `HelloWorldPlugin.dll` on Windows.
