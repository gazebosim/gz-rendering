// Milestone 2 sanity check: exercises the real gz-rendering Ogre2RenderEngine
// wayland/waylandDisplay/waylandSurface param-plumbing added this session,
// against a real (bare, roleless) wl_surface on the live compositor.
//
// Milestone 4 extension: also sets useCurrentGLContext=1, mirroring what
// Ogre2RenderEngine::CreateRenderWindow() actually sends in the real Gazebo
// case (params["externalGLControl"]="true"; params["currentGLContext"]="true"
// - see Ogre2RenderEngine.cc:1282-1285). To exercise that path meaningfully
// this harness now mimics Qt's own behaviour: it creates its own EGL context
// on a separate wl_surface and makes it current *before* calling
// gz::rendering::engine(), exactly like MinimalSceneRhiOpenGL.cc's
// context->makeCurrent(surface) immediately before GzRenderer::Initialize().
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/Scene.hh>
#include <gz/rendering/Camera.hh>
#include <cstdio>
#include <cstring>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

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

    // A SEPARATE surface for us (mimics Qt's own render-thread surface) and
    // one to hand to gz-rendering/Ogre (mimics the primary bootstrap window).
    wl_surface *ourSurface = wl_compositor_create_surface( g_compositor );
    wl_surface *surface = wl_compositor_create_surface( g_compositor );
    if( !ourSurface || !surface )
    {
        fprintf( stderr, "FAIL: wl_compositor_create_surface failed\n" );
        return 1;
    }
    printf( "OK: created bare wl_surfaces (no xdg_toplevel role)\n" );

    // ---- Set up our OWN EGL context and make it current, mirroring Qt ----
    EGLDisplay eglDisplay = eglGetPlatformDisplay( EGL_PLATFORM_WAYLAND_KHR, (void *)display, 0 );
    if( eglDisplay == EGL_NO_DISPLAY )
    {
        fprintf( stderr, "FAIL: eglGetPlatformDisplay failed\n" );
        return 1;
    }
    EGLint major = 0, minor = 0;
    eglInitialize( eglDisplay, &major, &minor );
    eglBindAPI( EGL_OPENGL_API );

    const EGLint configAttribs[] = { EGL_SURFACE_TYPE,   EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE,
                                      EGL_OPENGL_BIT,     EGL_RED_SIZE,   8,
                                      EGL_GREEN_SIZE,     8,              EGL_BLUE_SIZE,
                                      8,                  EGL_ALPHA_SIZE, 8,
                                      EGL_DEPTH_SIZE,     24,             EGL_STENCIL_SIZE,
                                      8,                  EGL_NONE };
    EGLConfig ourConfig;
    EGLint    numConfigs = 0;
    if( !eglChooseConfig( eglDisplay, configAttribs, &ourConfig, 1, &numConfigs ) || numConfigs == 0 )
    {
        fprintf( stderr, "FAIL: eglChooseConfig failed\n" );
        return 1;
    }

    wl_egl_window *ourEglWindow = wl_egl_window_create( ourSurface, 64, 64 );
    EGLSurface     ourEglSurface =
        eglCreatePlatformWindowSurface( eglDisplay, ourConfig, ourEglWindow, 0 );
    if( ourEglSurface == EGL_NO_SURFACE )
    {
        fprintf( stderr, "FAIL: eglCreatePlatformWindowSurface (ours) failed: 0x%x\n", eglGetError() );
        return 1;
    }

    const EGLint contextAttrs[] = { EGL_CONTEXT_MAJOR_VERSION,
                                     4,
                                     EGL_CONTEXT_MINOR_VERSION,
                                     5,
                                     EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                     EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
                                     EGL_NONE };
    EGLContext ourContext = eglCreateContext( eglDisplay, ourConfig, EGL_NO_CONTEXT, contextAttrs );
    if( ourContext == EGL_NO_CONTEXT )
    {
        fprintf( stderr, "FAIL: eglCreateContext (ours) failed: 0x%x\n", eglGetError() );
        return 1;
    }

    if( !eglMakeCurrent( eglDisplay, ourEglSurface, ourEglSurface, ourContext ) )
    {
        fprintf( stderr, "FAIL: eglMakeCurrent (ours) failed: 0x%x\n", eglGetError() );
        return 1;
    }
    printf( "OK: our own EGL 4.5 core context created and made current (mimics Qt)\n" );

    std::map<std::string, std::string> params;
    params["wayland"] = "1";
    params["waylandDisplay"] = std::to_string( reinterpret_cast<size_t>( display ) );
    params["waylandSurface"] = std::to_string( reinterpret_cast<size_t>( surface ) );
    params["useCurrentGLContext"] = "1";

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

    // Our own context must still be usable after the engine is torn down -
    // proves gz-rendering/Ogre never destroyed or terminated the adopted
    // context/display it doesn't own.
    if( !eglMakeCurrent( eglDisplay, ourEglSurface, ourEglSurface, ourContext ) )
    {
        fprintf( stderr,
                 "FAIL: our context is no longer valid after engine teardown: 0x%x\n",
                 eglGetError() );
        return 1;
    }
    printf( "OK: our context is still valid after engine teardown (correctly not owned)\n" );

    eglMakeCurrent( eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    eglDestroyContext( eglDisplay, ourContext );
    eglDestroySurface( eglDisplay, ourEglSurface );
    wl_egl_window_destroy( ourEglWindow );
    eglTerminate( eglDisplay );

    wl_surface_destroy( surface );
    wl_surface_destroy( ourSurface );
    wl_display_disconnect( display );

    printf( "\nALL CHECKS PASSED\n" );
    return 0;
}
