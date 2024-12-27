#pragma once
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "string"
#include "Utilities.hpp"

class Window {
protected:
	uint width, height;
	std::string title;
	bool running;
	SDL_Window* window;
	SDL_GLContext glContext;

public:
	Window(uint width, uint height, std::string title);
	~Window();

	template<class F> void Start(F start);
	template<class F> void Update(F update);

	getter(width)
	getter(height)
	getter(window)
};



template<class F> void Window::Start(F start) {
	// *** START-BODY *** //
	start();
}

template<class F> void Window::Update(F update) {
	uint currTime = 0;
	uint lastTime = SDL_GetTicks();

	SDL_Event Event;
	this->running = true;
	while (this->running) {
		currTime = SDL_GetTicks();
		while (SDL_PollEvent(&Event) != 0) {
			if (Event.type == SDL_QUIT) {
				this->running = false;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// *** UPDATE-BODY *** //
		float deltaTime = (currTime - lastTime) / 1000.0f;
		update(deltaTime);

		lastTime = currTime;
		SDL_GL_SwapWindow(this->window);
	}
}
