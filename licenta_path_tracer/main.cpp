#include "sstream"
#include "Window.hpp"
#include "gtx/transform.hpp"
#include <gtc/type_ptr.hpp>
#include "Camera.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "PointLight.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "RMOStructs.hpp"
#include "Cube.hpp"
#include "FX.hpp"
#include "Parent.hpp"
#include "UI.hpp"



GLuint compileShader(GLenum type, std::string source) {
	const char* search_dirs[] = { "/" };

	GLuint shader = glCreateShader(type);
	const char* csource = source.data();
	glShaderSource(shader, 1, &csource, NULL);

	glCompileShader(shader);
	/*
	if (includesHeaders == false) {
		glCompileShader(shader);
	} else {
		glCompileShaderIncludeARB(shader, 1, search_dirs, NULL);
	}
	*/

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	return shader;
}

GLuint createShaderProgram(std::string vertexSource, std::string fragmentSource) {
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

GLuint createComputeShaderProgram(std::string computeSource) {
	GLuint computeShader = compileShader(GL_COMPUTE_SHADER, computeSource);

	GLuint program = glCreateProgram();
	glAttachShader(program, computeShader);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(computeShader);
	return program;
}


static std::unordered_map<std::string, GLuint> buffers;
template<class CAST, class RMO> void sendCpuObjectsToGpu(GLuint shader_program, const std::vector<Object*> &objects, const std::string &array_name, uint buffer_index) {
	glUseProgram(shader_program);

	auto casts = get_of_type<CAST*>(objects);
	RMO* rmo_objs = new RMO[casts.size()];
	for (uint i = 0; i < casts.size(); i++) {
		rmo_objs[i] = *casts[i];
	}

	GLuint objs_ssbo;
	glGenBuffers(1, &objs_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, objs_ssbo);
	buffers[array_name] = objs_ssbo;

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(RMO) * casts.size(), rmo_objs, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer_index, objs_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glUniform1i(glGetUniformLocation(shader_program, (array_name + "_no").c_str()), casts.size());
	delete[] rmo_objs;
}

template<class CAST, class RMO> void updateGpuObjects(GLuint shader_program, const std::vector<Object*> &objects, const std::string &array_name) {
	glUseProgram(shader_program);

	auto casts = get_of_type<CAST*>(objects);
	RMO* rmo_objs = new RMO[casts.size()];
	for (uint i = 0; i < casts.size(); i++) {
		rmo_objs[i] = *casts[i];
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[array_name]);
	RMO* buffer_data = (RMO*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	if (buffer_data) {
		memcpy(buffer_data, rmo_objs, sizeof(RMO) * casts.size());
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	delete[] rmo_objs;
}


GLuint createTexture(uint width, uint height) {
	GLuint texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(
		GL_TEXTURE_2D,		// Target
		0,					// Mipmap level
		GL_RGBA32F,			// Internal format: 32-bit floating point per channel
		width,				// Texture width
		height,				// Texture height
		0,					// Border (must be 0)
		GL_RGBA,			// Format of the pixel data
		GL_FLOAT,			// Data type
		nullptr				// No initial data
	);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texId;
}

GLuint createBoundTexture(uint width, uint height, uint texIndex) {
	GLuint texId;
	glCreateTextures(GL_TEXTURE_2D, 1, &texId);

	glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureParameteri(texId, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(texId, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTextureStorage2D(texId, 1, GL_RGBA32F, width, height);
	glBindImageTexture(texIndex, texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	return texId;
}

std::vector<float> flattenImage(const std::vector<std::vector<glm::vec4>>& image) {
	std::vector<float> output(image.size() * image[0].size() * 4, 0.0f);
	for (int y = 0; y < image.size(); y++) {
		for (int x = 0; x < image[0].size(); x++) {
			for (int k = 0; k < 4; k++) {
				output[(y * image[0].size() + x) * 4 + k] = image[y][x][k];
			}
		}
	}
	return output;
}

void setTextureData(GLuint texId, const std::vector<std::vector<glm::vec4>> &image) {
	glBindTexture(GL_TEXTURE_2D, texId);
	std::vector<float> flattenedData = flattenImage(image);

	glTexSubImage2D(
		GL_TEXTURE_2D,				// Target
		0,							// Mipmap level
		0, 0,						// Offset (x, y)
		image[0].size(),			// Width of the data
		image.size(),				// Height of the data
		GL_RGBA,					// Format of the pixel data
		GL_FLOAT,					// Data type
		flattenedData.data()		// Pointer to the pixel data
	);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void readFile(std::string filePath, std::string &output) {
	std::ifstream file(filePath);
	if (file.is_open()) {
		std::stringstream ss;
		ss << file.rdbuf();
		output = ss.str();
	}
	file.close();
}

void addGLSLHeaderToFileSystem(std::string headerPath) {
	std::string headerContents;
	readFile(headerPath, headerContents);

	headerPath = "/" + headerPath;
	glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, headerPath.data(), headerContents.size(), headerContents.data());
}



int main(int argc, char* argv[]) {
	// Create Window
	Window window = Window(640, 480, "CUDA Pathtracer");


	// Create Textures for Ray-marching and final Output
	GLuint raymarch_tex = createBoundTexture(window.width_get(), window.height_get(), 0);
	GLuint output_tex = createBoundTexture(window.width_get(), window.height_get(), 1);


	// Read Shader files
	std::string raymarch_compute;
	readFile("raymarch.comp", raymarch_compute);

	std::string denoiser_compute;
	readFile("denoiser.comp", denoiser_compute);

	std::string vertex;
	readFile("rtarget.vert", vertex);

	std::string fragment;
	readFile("rtarget.frag", fragment);


	// Read Shader Header files
	addGLSLHeaderToFileSystem("basic_shapes.comp");
	addGLSLHeaderToFileSystem("utils.comp");


	// Compile Shaders
	GLuint raymarch_program = createComputeShaderProgram(raymarch_compute);
	GLuint denoiser_program = createComputeShaderProgram(denoiser_compute);
	GLuint rtarget_program = createShaderProgram(vertex, fragment);

	#pragma region Create Render Target

	float rtarget_verts[] = {
		// Positions    // UVs
		-1.0f, -1.0f,   0.0f, 0.0f,
		 1.0f, -1.0f,   1.0f, 0.0f,
		 1.0f,  1.0f,   1.0f, 1.0f,

		-1.0f, -1.0f,   0.0f, 0.0f,
		 1.0f,  1.0f,   1.0f, 1.0f,
		-1.0f,  1.0f,   0.0f, 1.0f
	};

	GLuint rtargetVAO, rtargetVBO;
	glGenVertexArrays(1, &rtargetVAO);
	glGenBuffers(1, &rtargetVBO);

	glBindVertexArray(rtargetVAO);

	glBindBuffer(GL_ARRAY_BUFFER, rtargetVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rtarget_verts), rtarget_verts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	#pragma endregion


	// Setup Camera Rays
	Camera camera = Camera({
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f }
	});
	std::vector<Object*> objects;
	std::vector<Light*> lights;

	std::vector<Ray> rays(window.width_get() * window.height_get());
	auto proj = glm::perspectiveFovLH_ZO(glm::radians(60.0f), (float)window.width_get(), (float)window.height_get(), 0.1f, 1000.0f);


	#pragma region Create Objects

	objects.emplace_back(new Sphere(
		{
			{ 0.0f, 10.0f, 0.0f },
			{ 0.0f,  0.0f, 0.0f },
			{ 1.0f,  1.0f, 1.0f }
		},
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		5.0f
	));
	objects.emplace_back(new Sphere(
		{
			{  5.0f, 15.0f, 5.0f },
			{ 45.0f,  0.0f, 0.0f },
			{  1.0f,  1.0f, 1.0f }
		},
		{ 1.0f, 0.5f, 0.0f, 1.0f },
		10.0f
	));
	objects.emplace_back(new Cube(
		{
			{ 0.0f, 0.0f, -5.0f },
			{ 0.0f, 0.0f,  0.0f },
			{ 1.0f, 1.0f,  1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 100.0f, 100.0f, 1.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ 50.0f, 0.0f, 0.0f },
			{  0.0f, 0.0f, 0.0f },
			{  1.0f, 1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 100.0f, 100.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ -50.0f, 0.0f, 0.0f },
			{   0.0f, 0.0f, 0.0f },
			{   1.0f, 1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 100.0f, 100.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ 0.0f, 50.0f, 0.0f },
			{ 0.0f,  0.0f, 0.0f },
			{ 1.0f,  1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 100.0f, 1.0f, 100.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ -10.0f, -10.0f,  5.0f },
			{   0.0f,   0.0f, 45.0f },
			{   2.0f,   1.0f,  1.0f }
		},
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 10.0f, 10.0f, 10.0f }
	));
	objects.emplace_back(new Cylinder(
		{
			{ -30.0f, 0.0f, 5.0f },
			{   0.0f, 0.0f, 0.0f },
			{   1.0f, 2.0f, 1.0f }
		},
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		4.0f,
		12.0f
	));
	objects.emplace_back(new Cylinder(
		{
			{ -35.0f, 10.0f, 0.0f },
			{  45.0f, 45.0f, 0.0f },
			{   1.0f,  1.0f, 4.0f }
		},
		{ 0.0f, 1.0f, 0.7f, 1.0f },
		4.0f,
		6.0f
	));
	objects.emplace_back(new Cone(
		{
			{ 25.0f, -20.0f, 5.0f },
			{  0.0f, -45.0f, 0.0f },
			{  1.0f,   2.0f, 3.0f }
		},
		{ 1.0f, 0.1f, 0.2f, 1.0f },
		4.0f,
		6.0f
	));

	#pragma endregion

	// Print GPU specifications
	/*
	GLint maxShaderStorageBlocks;
	glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &maxShaderStorageBlocks);
	std::cout << "Max Compute Shader Storage Blocks: " << maxShaderStorageBlocks << std::endl;

	GLint maxStorageBlockSize;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxStorageBlockSize);
	std::cout << "Max Shader Storage Block Size: " << maxStorageBlockSize << " bytes" << std::endl;

	int asdf;
	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &asdf);
	*/


	// Create Blue-Noise and Set Shader Uniforms
	// Blue-noise Texture-01
	auto blueNoiseImg = generators::generateBlueNoise(window.width_get(), window.height_get());
	auto blueNoiseGS = generators::valuesToGrayscale(blueNoiseImg);

	GLuint blueNoiseTex01 = createTexture(window.width_get(), window.height_get());
	setTextureData(blueNoiseTex01, blueNoiseGS);

	// Blue-noise Texture-02
	blueNoiseImg = generators::generateBlueNoise(window.width_get(), window.height_get());
	blueNoiseGS = generators::valuesToGrayscale(blueNoiseImg);

	GLuint blueNoiseTex02 = createTexture(window.width_get(), window.height_get());
	setTextureData(blueNoiseTex02, blueNoiseGS);

	GLuint blueNoiseTex = blueNoiseTex01;


	glUseProgram(raymarch_program);
	glUniform1i(glGetUniformLocation(raymarch_program, "blueNoise"), 0);


	// Setup Ray-Sampling
	static constexpr uint max_samples = 64;
	static int samples = max_samples;
	auto reset_pathtracer = [&]() {
		glUseProgram(raymarch_program);
		glUniform1i(glGetUniformLocation(raymarch_program, "scene_change"), 1);
		samples = max_samples + 1;
		updateGpuObjects<Sphere, rmo::Sphere>(raymarch_program, objects, "spheres");
		updateGpuObjects<Cube, rmo::Cube>(raymarch_program, objects, "cubes");
		updateGpuObjects<Cylinder, rmo::Cylinder>(raymarch_program, objects, "cylinders");
		updateGpuObjects<Cone, rmo::Cone>(raymarch_program, objects, "cones");
	};


	// Application Start function
	window.Start([&]() {
		glClearColor(0.5f, 0.1f, 0.2f, 1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(raymarch_program);
		glUniformMatrix4fv(glGetUniformLocation(raymarch_program, "camera_proj"), 1, GL_FALSE, glm::value_ptr(proj));
		

		// Add Components to Objects
		auto boolean = new boolean::Boolean(objects[0], boolean::Type::Difference);
		objects[1]->components_getr().emplace_back(boolean);
		objects[0]->visible_set(false);

		auto cP01 = new Parent(objects[0], objects[1]);
		objects[0]->components_getr().emplace_back(cP01);


		// Send Object Data to Ray-Marching Shader
		sendCpuObjectsToGpu<Sphere, rmo::Sphere>(raymarch_program, objects, "spheres", 1);
		sendCpuObjectsToGpu<Cube, rmo::Cube>(raymarch_program, objects, "cubes", 2);
		sendCpuObjectsToGpu<Cylinder, rmo::Cylinder>(raymarch_program, objects, "cylinders", 3);
		sendCpuObjectsToGpu<Cone, rmo::Cone>(raymarch_program, objects, "cones", 4);


		// UI
		UI::uiGenFuncs.emplace_back([]() {
			ImGuiIO& imguiIO = ImGui::GetIO(); (void)imguiIO;
			ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

			ImGui::Begin("Context Menu");
			
			ImGui::Text("Rendering... %d/%d samples", std::min((int)max_samples, std::max(1, (int)max_samples - samples)), max_samples);

			ImGui::End();

			//ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				//counter++;
			//ImGui::SameLine();
			//ImGui::Text("counter = %d", counter);

			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / imguiIO.Framerate, imguiIO.Framerate);
			//ImGui::End();
		});
	});
	// Application Update function
	window.Update([&](float deltaTime) {

		// Update App Title with current FPS
		SDL_SetWindowTitle(window.window_get(), std::to_string(1.0f / deltaTime).c_str());


		// ****** UPDATE LOGIC ****** //

		// Compute and Send Camera View Matrix to Ray-Marching Shader
		glm::mat4 view = glm::lookAtLH(
			camera.transform_getr().location,
			camera.transform_getr().location + camera.forward_getr(),
			camera.up_getr()
		);
		glUseProgram(raymarch_program);
		glUniformMatrix4fv(glGetUniformLocation(raymarch_program, "camera_view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(raymarch_program, "camera_pos"), 1, glm::value_ptr(camera.transform_getr().location));


		#pragma region Input

		// Setup Camera Input
		static float speed = 25.0f;
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_I]) {
			objects[0]->transform_getr().location += glm::vec3(0.0f, 0.0f, 1.0f) * -speed * deltaTime;
			//std::cout << objects[0]->transform_getr().location << std::endl;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_O]) {
			objects[0]->transform_getr().location += glm::vec3(0.0f, 0.0f, 1.0f) * speed * deltaTime;
			//std::cout << objects[0]->transform_getr().location << std::endl;
			reset_pathtracer();
		}

		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_W]) {
			camera.transform_getr().location += camera.forward_getr() * speed * deltaTime;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_S]) {
			camera.transform_getr().location += camera.forward_getr() * -speed * deltaTime;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A]) {
			camera.transform_getr().location += camera.right_getr() * -speed * deltaTime;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D]) {
			camera.transform_getr().location += camera.right_getr() * speed * deltaTime;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_Q]) {
			camera.transform_getr().location += glm::vec3(0.0f, 0.0f, 1.0f) * -speed * deltaTime;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_E]) {
			camera.transform_getr().location += glm::vec3(0.0f, 0.0f, 1.0f) * speed * deltaTime;
			reset_pathtracer();
		}

		static float sensitivity = 2.0f;
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_UP]) {
			auto rotation = glm::mat4(1.0)
				* glm::rotate(-1.0f * sensitivity * deltaTime, camera.right_getr())
				;
			camera.right_getr() = glm::vec4(camera.right_getr(), 1.0) * rotation;
			camera.forward_getr() = glm::vec4(camera.forward_getr(), 1.0) * rotation;
			camera.up_getr() = glm::vec4(camera.up_getr(), 1.0) * rotation;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_DOWN]) {
			auto rotation = glm::mat4(1.0)
				* glm::rotate(1.0f * sensitivity * deltaTime, camera.right_getr())
				;
			camera.right_getr() = glm::vec4(camera.right_getr(), 1.0) * rotation;
			camera.forward_getr() = glm::vec4(camera.forward_getr(), 1.0) * rotation;
			camera.up_getr() = glm::vec4(camera.up_getr(), 1.0) * rotation;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LEFT]) {
			auto rotation = glm::mat4(1.0)
				* glm::rotate(-1.0f * sensitivity * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f))
				;
			camera.right_getr() = glm::vec4(camera.right_getr(), 1.0) * rotation;
			camera.forward_getr() = glm::vec4(camera.forward_getr(), 1.0) * rotation;
			camera.up_getr() = glm::vec4(camera.up_getr(), 1.0) * rotation;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RIGHT]) {
			auto rotation = glm::mat4(1.0)
				* glm::rotate(1.0f * sensitivity * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f))
				;
			camera.right_getr() = glm::vec4(camera.right_getr(), 1.0) * rotation;
			camera.forward_getr() = glm::vec4(camera.forward_getr(), 1.0) * rotation;
			camera.up_getr() = glm::vec4(camera.up_getr(), 1.0) * rotation;
			reset_pathtracer();
		}


		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_N]) {
			objects[1]->transform_getr().scale *= 0.9f;
			dynamic_cast<Parent*>(*objects[0]->components_getrc().rbegin())->applyTransform();
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_M]) {
			objects[1]->transform_getr().scale *= 1.1f;
			dynamic_cast<Parent*>(*objects[0]->components_getrc().rbegin())->applyTransform();
			reset_pathtracer();
		}

		#pragma endregion


		// Create Lights
		static glm::vec3 light_pos(-20.0, -20.0, 20.0);



		// ****** UPDATE RENDERING ****** //

		// Compute Path-Tracing Samples
		if (samples > 0) {
			glUseProgram(raymarch_program);
			glUniform1i(glGetUniformLocation(raymarch_program, "rng_seed"), random(1, 1000));
			glUniform1i(glGetUniformLocation(raymarch_program, "samples"), max_samples);
			glUniform3fv(glGetUniformLocation(raymarch_program, "light_pos"), 1, glm::value_ptr(light_pos));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, blueNoiseTex);
			blueNoiseTex = blueNoiseTex == blueNoiseTex01 ? blueNoiseTex02 : blueNoiseTex01;

			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			samples--;
		}
		else if (samples == 0) {
			//std::cout << "Render finished..." << std::endl;
			samples--;
		}


		// Denoiser
		static constexpr uint denoisingPasses = 1;
		glUseProgram(denoiser_program);
		glUniform1i(glGetUniformLocation(denoiser_program, "samples"), std::max((int)max_samples - samples, 1));
		for (uint i = 0; i < denoisingPasses; i++) {
			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
		/*
		// Denoiser Blur-Pass
		glUniform1i(glGetUniformLocation(denoiser_program, "blur"), 0);
		glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glUniform1i(glGetUniformLocation(denoiser_program, "blur"), 1);
		*/


		// Render final Output to Screen
		glUseProgram(rtarget_program);
		glBindVertexArray(rtargetVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, output_tex);

		glUniform1i(glGetUniformLocation(rtarget_program, "tex_output"), 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);


		// Initiate Path-Tracing Sampling process
		if (samples == max_samples) {
			glUseProgram(raymarch_program);

			glUniform1i(glGetUniformLocation(raymarch_program, "scene_change"), -1);
			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glUniform1i(glGetUniformLocation(raymarch_program, "scene_change"), 0);
		}

	});

	return 0;
}
