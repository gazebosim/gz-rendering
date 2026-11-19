# Vendored OGRE ShadowVolume media

These files are copied **verbatim** from OGRE's `Media/ShadowVolume` directory.

* Source: https://github.com/OGRECave/ogre
* Version: `v1.12.10`

OGRE 1.12 made the stencil-shadow programs a hard requirement at engine
initialisation: `OgreShadowVolumeExtrudeProgram::initialise()` looks up the
`Ogre/ShadowExtrude*` / `Ogre/ShadowBlend*` programs in a resource location
named `ShadowVolume`. Rather than registering the OGRE package `Media` tree at
runtime (which risks loading a different version of these scripts than the one
gz-rendering was tested with), we keep a pinned copy here, alongside the other
OGRE media we vendor (see `../rtshaderlib150`).

When bumping the OGRE dependency, re-sync these files from the matching upstream
tag and update the version above.
