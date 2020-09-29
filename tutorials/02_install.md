\page installation Installation

[Install](#install)

* [Binary Install](#binary-install)

* [Source Install](#source-install)

    * [Prerequisites](#prerequisites)

    * [Building from Source](#building-from-source)

We recommend following the [Binary Install](#binary-install) instructions to get up and running as quickly and painlessly as possible.

The [Source Install](#source-install) instructions should be used if you need the very latest software improvements, you need to modify the code, or you plan to make a contribution.

## Binary Install

### Ubuntu

Setup your computer to accept software from packages.osrfoundation.org:

    sudo apt -y install wget lsb-release gnupg
    sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
    wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
    sudo apt update

Install Ignition Rendering:

    # This installs ign-rendering3. Change the number after libignition-rendering to the version you want
    sudo apt install libignition-rendering3-dev

## Source Install

### Prerequisites

#### Ubuntu Bionic 18.04 or above

Install dependencies:

    sudo apt -y install wget lsb-release gnupg
    sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
    wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
    sudo apt update
    sudo apt install -y \
        g++-8 \
        cmake \
        pkg-config \
        git \
        libglew-dev  \
        libfreeimage-dev \
        freeglut3-dev \
        libxmu-dev \
        libxi-dev \
        libignition-cmake2-dev \
        libignition-math6-dev \
        libignition-common3-dev \
        libignition-plugin-dev
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8 --slave /usr/bin/gcov gcov /usr/bin/gcov-8

Clone source code

    # This checks out the `master` branch. You can append `-b ign-rendering#` (replace # with a number) to checkout a specific version
    git clone http://github.com/ignitionrobotics/ign-rendering

#### Supported Rendering Engines

Ignition Rendering will look for rendering libraries installed in the system and
build the relevant plugins if dependencies are found.

**OGRE 1.x**

    # this installs ogre 1.9. Alternatively, you can install 1.8
    sudo apt-get install libogre-1.9-dev

**OGRE 2.x (supported in Versions >= ign-rendering1)**

Add OSRF packages if you have not done so already:

    sudo apt -y install wget lsb-release gnupg
    sudo sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
    wget http://packages.osrfoundation.org/gazebo.key -O - | sudo apt-key add -
    sudo apt update

Install OGRE 2.1 debs

    sudo apt install libogre-2.1-dev

**OptiX (experimental)**

Download and install by following instructions on NVIDIA website

CUDA:

http://docs.nvidia.com/cuda

OptiX:

https://developer.nvidia.com/optix

Update `LD_LIBRARY_PATH` and add an `OPTIX_INSTALL_DIR` environment variables
so that ign-rendering can find Optix, e.g. if you installed version 4.0.2 in HOME/optix:

    export LD_LIBRARY_PATH=${HOME}/optix/NVIDIA-OptiX-SDK-4.0.2-linux64/lib64:${LD_LIBRARY_PATH}
    export OPTIX_INSTALL_DIR=${HOME}/optix/NVIDIA-OptiX-SDK-4.0.2-linux64

Note: If you encounter errors about different exception specifiers in optix math
when building Ign Rendering OptiX plugin, edit
`[optix_install_dir]/include/optixu/optixu_math_namespace.h` and comment
out the section that defines `fminf`, fmaxf, and `copysignf` (for optix
sdk 4.0.2, comment out lines 167-206).


### Building from source

Build and install as follows:

    cd ign-rendering
    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install/dir
    make -j4
    make install

Replace `/path/to/install/dir` to whatever directory you want to install this package to
