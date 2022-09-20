# LuxCore Render Engine 

This creates an example plugin using the LuxCore render engine. The implementation is incomplete and only supports basic scenes. Some parameters of the engine are hardcoded and can be adjusted for different effects:

https://wiki.luxcorerender.org/LuxCore_SDL_Reference_Manual_v2.5

## How to build and run the example
Download the LuxCoreSDK using the following link (make sure to download the SDK and not the standalone): https://luxcorerender.org/download/
1. Build and install ign-rendering:
    ~~~
    cd ign-rendering
    mkdir build
    cd build
    cmake ..
    make install
    ~~~

2. Build the example:
    ~~~
    export LUXCORE_SDK_PATH=<PATH_TO_LUXCORE_SDK_DIRECTORY>
    cd ../examples/lux_core_engine
    mkdir build
    cd build
    cmake ..
    make
    ~~~
3. Run the example:

    ~~~
    export IGN_RENDERING_PLUGIN_PATH=$PWD
    ./simple_demo
    ~~~