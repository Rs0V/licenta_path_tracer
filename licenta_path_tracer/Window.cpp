#include "iostream"
#include "Window.hpp"


Window::Window(uint width, uint height, std::string title)
	:
	width(width),
	height(height),
	title(title),
	running(false),
	window(nullptr),
	glContext(nullptr)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "Error initializing SDL2: " << SDL_GetError() << std::endl;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_Window* window = SDL_CreateWindow(
		title.data(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	);
	if (window == nullptr) {
		std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
		SDL_Quit();
	}
	this->window = window;

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (glContext == nullptr) {
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(this->window);
		SDL_Quit();
	}


	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &imguiIO = ImGui::GetIO(); (void)imguiIO;

	imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	const char* glsl_version = "#version 460 core";
	ImGui_ImplSDL2_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init(glsl_version);


	#if USE_GLAD == true
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "Error initializing GLAD!" << std::endl;
		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(this->window);
		SDL_Quit();
	}
	#else
	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		std::cerr << "Error initializing GLEW: " << glewGetErrorString(glewStatus) << std::endl;
		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(this->window);
		SDL_Quit();
	}
	#endif

	SDL_GL_SetSwapInterval(1);
}

Window::~Window() {
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(this->glContext);
	SDL_DestroyWindow(this->window);
	SDL_Quit();
}
