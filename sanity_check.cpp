#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/Camera.hh>
#include <cstdio>

int main()
{
    std::map<std::string, std::string> params;
    params["headless"] = "1";

    gz::rendering::RenderEngine *engine = gz::rendering::engine(
        "ogre2", params, "/home/ae/Work/work_ws/gz-deps-install/lib/gz-rendering/engine-plugins" );
    if( !engine )
    {
        fprintf( stderr, "FAIL: could not load ogre2 engine\n" );
        return 1;
    }
    printf( "OK: ogre2 engine loaded\n" );

    gz::rendering::ScenePtr scene = engine->CreateScene( "sanity_scene" );
    if( !scene )
    {
        fprintf( stderr, "FAIL: could not create scene\n" );
        return 1;
    }
    printf( "OK: scene created\n" );

    gz::rendering::CameraPtr camera = scene->CreateCamera( "sanity_camera" );
    if( !camera )
    {
        fprintf( stderr, "FAIL: could not create camera\n" );
        return 1;
    }
    camera->SetImageWidth( 64 );
    camera->SetImageHeight( 64 );
    printf( "OK: camera created (%ux%u)\n", camera->ImageWidth(), camera->ImageHeight() );

    camera->PreRender();
    camera->Update();
    printf( "OK: camera PreRender()+Update() succeeded\n" );

    scene->Destroy();
    gz::rendering::unloadEngine( engine->Name() );
    printf( "\nALL CHECKS PASSED\n" );
    return 0;
}
