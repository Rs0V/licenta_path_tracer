#pragma once
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "string"
#include "Utilities.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "UI.hpp"


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
			ImGui_ImplSDL2_ProcessEvent(&Event);
			if (Event.type == SDL_QUIT) {
				this->running = false;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// *** UPDATE-BODY *** //
		float deltaTime = (currTime - lastTime) / 1000.0f;
		update(deltaTime);

		lastTime = currTime;
		//SDL_GL_SwapWindow(this->window);



		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();


		UI::CreateUI();
		

		// Rendering
		ImGuiIO &imguiIO = ImGui::GetIO(); (void)imguiIO;
		ImGui::Render();
		glViewport(0, 0, (int)imguiIO.DisplaySize.x, (int)imguiIO.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(this->window);
	}
}
