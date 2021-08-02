# Engine plugin

This is a proof of concept for using LuxCore rendering (https://luxcorerender.org/) engine as a plugin for ign-rendering.

## Build

From https://luxcorerender.org/download/ download the latests version of LuxCore API SDK. This PR was created with v2.5, but 
newer versions should work if the engine remains backwards compatible.

From the root of the `ign-rendering` repository, do the following to build the POC:

~~~
cd examples/luxcore

modify the CMakeLists.txt lines 14 and 15 to point to your luxcore sdk folder:
include_directories("PATH_TO_LUXCORE_SDK_FOLDER/include")
link_directories("PATH_TO_LUXCORE_SDK_FOLDER/lib")

mkdir build
cd build
cmake ..
make
~~~

This will generate the `LuxCorePlugin` library under `build`.
The exact name of the library file depends on the operating system
such as `libLuxCorePlugin.so` on Linux, `libLuxCorePlugin.dylib` on macOS,
and `LuxCorePlugin.dll` on Windows. This was only tested on Linux.

## Run

Be sure to have the `IGN_RENDERING_PLUGIN_PATH` environment variable set to the path
where your plugin is located.  From within the `build` directory of this example, you can run

~~~
export IGN_RENDERING_PLUGIN_PATH=$PWD
~~~

to set the environment variable accordingly.

This implementation was thought for use with the simple_demo example.

From the root of the `ign-rendering` repository:

cd examples/simple_demo
mkdir build
cd build
cmake ..
make
./simple_demo

Also, line 463 could couse issues, so changing that path to PATH_TO_LUXCORE_SDK_FOLDER/scenes/cornell/cornell.scn 
and rebuilding if you see an issue on runtime

## State of the branch and known issues

The only part missing AFAIK is the camera->capture part. I laid down the connecting with luxcore, but line 49
in LuxCore.cc is failling and I was not able to find why.

Im not sure if line 89 works as intended, as a was not able to reach that call because of the forementioned issue. According to
luxcore documentation, it should.

Line 463 is a hack, hardcoded to a file. For some reason luxcore always needs a scene file, maybe pointing it to a empty file
works, but I was not able to test that.

Luxcore works representing the scene as a Properties object, so my idea was to share that object amoung the luxcore 
pluggin and letting the ign-rendering classes modify that whenever needed.