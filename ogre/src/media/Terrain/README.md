# Vendored OGRE Terrain media

These files are copied **verbatim** from OGRE's `Media/Terrain` directory.

* Source: https://github.com/OGRECave/ogre
* Version: `v1.12.10`

The stock `TerrainMaterialGeneratorA` SM2Profile generates GLSL that
`#include`s `TerrainTransforms.glsl` (vertex) and `TerrainHelpers.glsl`
(fragment); both also rely on `OgreUnifiedShader.h`, vendored under
`../ShadowVolume`. Rather than registering the OGRE package `Media` tree at
runtime (which risks loading a different version of these scripts than the one
gz-rendering was tested with), we keep a pinned copy here, alongside the other
OGRE media we vendor (see `../rtshaderlib150`).

When bumping the OGRE dependency, re-sync these files from the matching upstream
tag and update the version above.
