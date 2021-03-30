/* Using standard C++ output libraries */
#include <cstdlib>
#include <iostream>
// using namespace std;
//
// /* Use glew.h instead of gl.h to get all the GL prototypes declared */
// #include <GL/glew.h>
// /* Using SDL2 for the base window and OpenGL context init */
// #include <SDL2/SDL.h>
// /* ADD GLOBAL VARIABLES HERE LATER */
//
// bool init_resources(void) {
//   /* FILLED IN LATER */
//   return true;
// }
//
// void render(SDL_Window* window) {
//   /* FILLED IN LATER */
// }
//
// void free_resources() {
//   /* FILLED IN LATER */
// }
//
// void mainLoop(SDL_Window* window) {
// 	while (true) {
// 		SDL_Event ev;
// 		while (SDL_PollEvent(&ev)) {
// 			if (ev.type == SDL_QUIT)
// 				return;
// 		}
// 		render(window);
// 	}
// }

#include <windows.h>

// int WINAPI WinMain(HINSTANCE hInstance,
//                    HINSTANCE hPrevInstance,
//                    LPSTR lpCmdLine,
//                    int nCmdShow)
// {
int main(){
  std::cout << "lol" << '\n';
	// /* SDL-related initialising functions */
	// SDL_Init(SDL_INIT_VIDEO);
	// SDL_Window* window = SDL_CreateWindow("My First Triangle",
	// 	SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	// 	640, 480,
	// 	SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	// SDL_GL_CreateContext(window);
  //
	// /* Extension wrangler initialising */
	// GLenum glew_status = glewInit();
	// if (glew_status != GLEW_OK) {
	// 	cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
	// 	return EXIT_FAILURE;
	// }
  //
	// /* When all init functions run without errors,
	//    the program can initialise the resources */
	// if (!init_resources())
	// 	return EXIT_FAILURE;
  //
	// /* We can display something if everything goes OK */
	// mainLoop(window);
  //
	// /* If the program exits in the usual way,
	//    free resources and exit with a success */
	// free_resources();
	return EXIT_SUCCESS;
}
