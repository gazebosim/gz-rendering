// Milestone 2 sanity check: exercises the real gz-rendering Ogre2RenderEngine
// wayland/waylandDisplay/waylandSurface param-plumbing added this session,
// against a real (bare, roleless) wl_surface on the live compositor.
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/Camera.hh>
#include <cstdio>
#include <cstring>

#include <wayland-client.h>

static wl_compositor *g_compositor = nullptr;

static void registry_global( void *, wl_registry *registry, uint32_t name, const char *interface,
                              uint32_t )
{
    if( strcmp( interface, "wl_compositor" ) == 0 )
        g_compositor = static_cast<wl_compositor *>(
            wl_registry_bind( registry, name, &wl_compositor_interface, 4 ) );
}
static void registry_global_remove( void *, wl_registry *, uint32_t ) {}
static const wl_registry_listener registry_listener = { registry_global, registry_global_remove };

int main()
{
    wl_display *display = wl_display_connect( nullptr );
    if( !display )
    {
        fprintf( stderr, "FAIL: wl_display_connect failed\n" );
        return 1;
    }
    printf( "OK: connected to wl_display\n" );

    wl_registry *registry = wl_display_get_registry( display );
    wl_registry_add_listener( registry, &registry_listener, nullptr );
    wl_display_roundtrip( display );

    if( !g_compositor )
    {
        fprintf( stderr, "FAIL: wl_compositor not found\n" );
        return 1;
    }

    wl_surface *surface = wl_compositor_create_surface( g_compositor );
    if( !surface )
    {
        fprintf( stderr, "FAIL: wl_compositor_create_surface failed\n" );
        return 1;
    }
    printf( "OK: created bare wl_surface (no xdg_toplevel role)\n" );

    std::map<std::string, std::string> params;
    params["wayland"] = "1";
    params["waylandDisplay"] = std::to_string( reinterpret_cast<size_t>( display ) );
    params["waylandSurface"] = std::to_string( reinterpret_cast<size_t>( surface ) );

    gz::rendering::RenderEngine *engine = gz::rendering::engine(
        "ogre2", params, "/home/ae/Work/work_ws/gz-deps-install/lib/gz-rendering/engine-plugins" );
    if( !engine )
    {
        fprintf( stderr, "FAIL: could not load ogre2 engine in wayland mode\n" );
        return 1;
    }
    printf( "OK: ogre2 engine loaded via native Wayland EGL path\n" );

    gz::rendering::ScenePtr scene = engine->CreateScene( "wayland_sanity_scene" );
    if( !scene )
    {
        fprintf( stderr, "FAIL: could not create scene\n" );
        return 1;
    }
    printf( "OK: scene created\n" );

    gz::rendering::CameraPtr camera = scene->CreateCamera( "wayland_sanity_camera" );
    if( !camera )
    {
        fprintf( stderr, "FAIL: could not create camera\n" );
        return 1;
    }
    camera->SetImageWidth( 64 );
    camera->SetImageHeight( 64 );
    printf( "OK: camera created (%ux%u)\n", camera->ImageWidth(), camera->ImageHeight() );

    for( int i = 0; i < 30; ++i )
    {
        camera->PreRender();
        camera->Update();
        wl_display_flush( display );
        wl_display_dispatch_pending( display );
    }
    printf( "OK: 30 PreRender()+Update() iterations succeeded\n" );

    scene->Destroy();
    gz::rendering::unloadEngine( engine->Name() );
    printf( "OK: engine unloaded cleanly\n" );

    wl_surface_destroy( surface );
    wl_display_disconnect( display );

    printf( "\nALL CHECKS PASSED\n" );
    return 0;
}
