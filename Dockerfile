# docker-compose build --build-arg BASE_IMAGE=ghcr.io/deepx-inc/base_images --build-arg TAG=foxy
# Adds DeepX Gazebo Rendering image.
# 
# TODO: Add comment about command to push to repo
#
# Arguments:
# - BASE_IMAGE: Base image.
# - TAG: Tag of the BASE_IMAGE.


ARG BASE_IMAGE
ARG TAG
ARG FROM_SOURCE
FROM ${BASE_IMAGE}:${TAG} AS DEEPX_GZ_RENDERING
# FROM ubuntu:20.04

# ---BUILD TOOLS---
# RUN apt -qq update \
#     && apt -qq install -y --no-install-recommends \
#     cmake \
#     ffmpeg \
#     freeglut3-dev \
#     g++-8 \
#     git \
#     gnupg \
#     libfreeimage-dev \
#     libglew-dev \
#     libogre-2.1-dev \
#     libxi-dev \
#     libxmu-dev \
#     ninja-build \
#     pkg-config \
#     redis-server \
#     redis-tools \
#     software-properties-common \
#     lsb-release \
#     wget \
#     curl

# ---Temporary solution to stop flikering in gazebo
# Before mesa version 22.0.2, there was no flickering issue.
# After doing apt update && apt upgrade, the flickering appears.
# This flickering is observed in mesa version 22.1.1
# [TODO] Remove this apt-mark hold, after the issue is solved.
# RUN apt-mark hold \
#     libegl-mesa0 \
#     libegl1-mesa-dev \
#     libgl1-mesa-dev \
#     libgl1-mesa-dri \
#     libglapi-mesa \
#     libgles2-mesa-dev \
#     libglu1-mesa-dev \
#     libglu1-mesa \
#     libglx-mesa0 \
#     mesa-utils

# ---STANDARD GAZEBO INSTALL---
# RUN apt -qq update && \
#     sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list' && \
#     sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-prerelease `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-prerelease.list' && \
#     wget http://packages.osrfoundation.org/gazebo.key -O - | apt-key add - && \
#     add-apt-repository ppa:kisak/kisak-mesa && \
#     curl -sSL http://get.gazebosim.org | sh && \
#     update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8 --slave /usr/bin/gcov gcov /usr/bin/gcov-8 && \
#     apt -qq update && \
#     apt -qq upgrade -y --no-install-recommends \
#     libignition-cmake2-dev \
#     libignition-common4-dev \
#     libignition-fuel-tools7-dev \
#     libignition-gazebo6-dev \
#     libignition-gui6-dev \
#     libignition-launch5-dev \
#     libignition-math6-dev \
#     libignition-msgs8-dev \
#     libignition-physics5-dev \
#     libignition-plugin-dev \
#     libignition-tools-dev \
#     libignition-transport11-dev \
#     libignition-utils-dev \
#     libsdformat12 \
#     libignition-plugin-dev \
#     && apt -qq -y autoclean \
#     && apt -qq -y autoremove \
#     && rm -rf /var/lib/apt/lists/*

WORKDIR "/root/gazebo"

COPY . gz-rendering/

# RUN mkdir -p gz-rendering/build && \
#     cd gz-rendering/build && \
#     cmake .. && \ 
#     make install

RUN mkdir -p gz-rendering/build && \
    cd gz-rendering/build