file(GLOB files "*.metal" "*.hlsl" "*.vert" "*.program" "*.dds" "*.frag" "*.h")

install(FILES ${files} DESTINATION ${IGN_RENDERING_RESOURCE_PATH}/ogre/media/materials/ShadowVolume)
