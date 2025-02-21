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

template<class CAST, class RMO> void updateGpuObjects(GLuint shader_program, const std::vector<Object*>& objects, const std::string& array_name) {
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


void createTexture(GLuint &texId, uint width, uint height, uint texIndex) {
	glCreateTextures(GL_TEXTURE_2D, 1, &texId);

	glTexParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(texId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(texId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTextureStorage2D(texId, 1, GL_RGBA32F, width, height);
	glBindImageTexture(texIndex, texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
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
	GLuint raymarch_tex;
	createTexture(raymarch_tex, window.width_get(), window.height_get(), 0);

	GLuint output_tex;
	createTexture(output_tex, window.width_get(), window.height_get(), 1);


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
			{ 5.0f, 15.0f, 5.0f },
			{ 0.0f,  0.0f, 0.0f },
			{ 1.0f,  1.0f, 1.0f }
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
			{ 0.0f, 0.0f, 0.0f },
			{ 1.0f, 1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 100.0f, 100.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ -50.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 1.0f, 1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 100.0f, 100.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ 0.0f, 50.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 1.0f, 1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 100.0f, 1.0f, 100.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ -10.0f, -10.0f, 5.0f },
			{   0.0f,   0.0f, 0.0f },
			{   1.0f,   1.0f, 1.0f }
		},
		{ 0.0f, 0.0f, 1.0f, 1.0f },
		{ 10.0f, 10.0f, 10.0f }
	));
	objects.emplace_back(new Cylinder(
		{
			{ -30.0f, 0.0f, 5.0f },
			{   0.0f, 0.0f, 0.0f },
			{   1.0f, 1.0f, 1.0f }
		},
		{ 0.0f, 1.0f, 0.0f, 1.0f },
		4.0f,
		12.0f
	));
	objects.emplace_back(new Cylinder(
		{
			{ -35.0f, 10.0f, 0.0f },
			{   0.0f, 0.0f, 0.0f },
			{   1.0f, 1.0f, 1.0f }
		},
		{ 0.0f, 1.0f, 0.7f, 1.0f },
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
	std::cout << asdf << std::endl;
	*/


	// Application Start function
	window.Start([&]() {
		glClearColor(0.5f, 0.1f, 0.2f, 1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(raymarch_program);
		glUniformMatrix4fv(glGetUniformLocation(raymarch_program, "camera_proj"), 1, GL_FALSE, glm::value_ptr(proj));
		
		// Send Object Data to Ray-Marching Shader
		sendCpuObjectsToGpu<Sphere, rmo::Sphere>(raymarch_program, objects, "spheres", 1);
		sendCpuObjectsToGpu<Cube, rmo::Cube>(raymarch_program, objects, "cubes", 2);
		sendCpuObjectsToGpu<Cylinder, rmo::Cylinder>(raymarch_program, objects, "cylinders", 3);
	});
	// Application Update function
	window.Update([&](float deltaTime) {

		// Update App Title with current FPS
		SDL_SetWindowTitle(window.window_get(), std::to_string(1.0f / deltaTime).c_str());

		// Setup Ray-Sampling
		static constexpr uint max_samples = 1024;
		static int samples = max_samples;
		auto reset_pathtracer = [&]() {
			glUniform1i(glGetUniformLocation(raymarch_program, "reset"), 1);
			samples = max_samples + 1;
			updateGpuObjects<Sphere, rmo::Sphere>(raymarch_program, objects, "spheres");
			updateGpuObjects<Cube, rmo::Cube>(raymarch_program, objects, "cubes");
			updateGpuObjects<Cylinder, rmo::Cylinder>(raymarch_program, objects, "cylinders");
		};



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


		// Setup Camera Input
		static float speed = 25.0f;
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_I]) {
			objects[0]->transform_getr().location += glm::vec3(0.0f, 0.0f, 1.0f) * -speed * deltaTime;
			std::cout << objects[0]->transform_getr().location << std::endl;
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_O]) {
			objects[0]->transform_getr().location += glm::vec3(0.0f, 0.0f, 1.0f) * speed * deltaTime;
			std::cout << objects[0]->transform_getr().location << std::endl;
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


		// Create Lights
		static glm::vec3 light_pos(-20.0, -20.0, 20.0);



		// ****** UPDATE RENDERING ****** //

		// Compute Path-Tracing Samples
		if (samples > 0) {
			glUseProgram(raymarch_program);
			glUniform1i(glGetUniformLocation(raymarch_program, "rng_seed"), random(1, 1000));
			glUniform1i(glGetUniformLocation(raymarch_program, "samples"), max_samples);
			glUniform3fv(glGetUniformLocation(raymarch_program, "light_pos"), 1, glm::value_ptr(light_pos));

			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			samples--;
		}
		else if (samples == 0) {
			std::cout << "Render finished..." << std::endl;
			samples--;
		}


		// Denoiser Sharpen-Pass
		static constexpr uint denoisingPasses = 64;
		glUseProgram(denoiser_program);
		glUniform1i(glGetUniformLocation(denoiser_program, "samples"), max_samples);
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
		if (samples == max_samples) { // Add random positive integer to right operand of comparison in order to disable sampling
			glUseProgram(raymarch_program);
			glUniform1i(glGetUniformLocation(raymarch_program, "reset"), -1);

			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glUniform1i(glGetUniformLocation(raymarch_program, "reset"), 0);
		}
	});

	return 0;
}
