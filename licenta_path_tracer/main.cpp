#include "iostream"
#include "fstream"
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



GLuint compileShader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	return shader;
}

GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
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

GLuint createComputeShaderProgram(const char* computeSource) {
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

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(RMO) * casts.size(), rmo_objs, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer_index, objs_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glUniform1i(glGetUniformLocation(shader_program, (array_name + "_no").c_str()), casts.size());
	delete[] rmo_objs;
}


int main(int argc, char* argv[]) {
	Window window = Window(640, 480, "CUDA Pathtracer");

	GLuint raymarch_tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &raymarch_tex);

	glTexParameteri(raymarch_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(raymarch_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(raymarch_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(raymarch_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTextureStorage2D(raymarch_tex, 1, GL_RGBA32F, window.width_get(), window.height_get());
	glBindImageTexture(0, raymarch_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	std::ifstream file("raymarch.comp");
	std::string compute;
	if (file.is_open()) {
		std::stringstream ss;
		ss << file.rdbuf();
		compute = ss.str();
	}
	file.close();

	file = std::ifstream("rtarget.vert");
	std::string vertex;
	if (file.is_open()) {
		std::stringstream ss;
		ss << file.rdbuf();
		vertex = ss.str();
	}
	file.close();

	file = std::ifstream("rtarget.frag");
	std::string fragment;
	if (file.is_open()) {
		std::stringstream ss;
		ss << file.rdbuf();
		fragment = ss.str();
	}
	file.close();

	auto raymarch_program = createComputeShaderProgram(compute.c_str());
	auto rtarget_program = createShaderProgram(vertex.c_str(), fragment.c_str());

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



	Camera camera = Camera({
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f }
	});
	std::vector<Object*> objects;
	std::vector<Light*> lights;

	std::vector<Ray> rays(window.width_get() * window.height_get());
	auto proj = glm::perspectiveFovLH_ZO(glm::radians(60.0f), (float)window.width_get(), (float)window.height_get(), 0.1f, 1000.0f);


	objects.emplace_back(new Sphere(
		{
			{ 0.0f, 10.0f, 0.0f },
			{ 0.0f,  0.0f, 0.0f },
			{ 1.0f,  1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		5.0f
	));
	objects.emplace_back(new Sphere(
		{
			{ 5.0f, 15.0f, 5.0f },
			{ 0.0f,  0.0f, 0.0f },
			{ 1.0f,  1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
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
	objects.emplace_back(new Cylinder(
		{
			{ -30.0f, 0.0f, 5.0f },
			{   0.0f, 0.0f, 0.0f },
			{   1.0f, 1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		4.0f,
		12.0f
	));
	objects.emplace_back(new Cube(
		{
			{ -10.0f, -10.0f, 5.0f },
			{   0.0f,   0.0f, 0.0f },
			{   1.0f,   1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 10.0f, 10.0f, 10.0f }
	));
	objects.emplace_back(new Cylinder(
		{
			{ -35.0f, 10.0f, 0.0f },
			{   0.0f, 0.0f, 0.0f },
			{   1.0f, 1.0f, 1.0f }
		},
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		4.0f,
		6.0f
	));

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

	window.Start([&]() {
		glClearColor(0.5f, 0.1f, 0.2f, 1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(raymarch_program);
		glUniformMatrix4fv(glGetUniformLocation(raymarch_program, "camera_proj"), 1, GL_FALSE, glm::value_ptr(proj));
		
		sendCpuObjectsToGpu<Sphere, rmo::Sphere>(raymarch_program, objects, "spheres", 1);
		sendCpuObjectsToGpu<Cube, rmo::Cube>(raymarch_program, objects, "cubes", 2);
		sendCpuObjectsToGpu<Cylinder, rmo::Cylinder>(raymarch_program, objects, "cylinders", 3);
	});
	window.Update([&](float deltaTime) {
		SDL_SetWindowTitle(window.window_get(), std::to_string(1.0f / deltaTime).c_str());


		// ****** UPDATE LOGIC ****** //

		glm::mat4 view = glm::lookAtLH(
			camera.transform_getr().location,
			camera.transform_getr().location + camera.forward_getr(),
			camera.up_getr()
		);
		glUseProgram(raymarch_program);
		glUniformMatrix4fv(glGetUniformLocation(raymarch_program, "camera_view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(raymarch_program, "camera_pos"), 1, glm::value_ptr(camera.transform_getr().location));


		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_I]) {
			objects[0]->transform_getr().location += glm::vec3(0.0f, 10.0f, 0.0f) * deltaTime;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_O]) {
			objects[0]->transform_getr().location += glm::vec3(0.0f, -10.0f, 0.0f) * deltaTime;
		}

		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_W]) {
			camera.transform_getr().location += camera.forward_getr() * 10.0f * deltaTime;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_S]) {
			camera.transform_getr().location += camera.forward_getr() * -10.0f * deltaTime;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A]) {
			camera.transform_getr().location += camera.right_getr() * -10.0f * deltaTime;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D]) {
			camera.transform_getr().location += camera.right_getr() * 10.0f * deltaTime;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_Q]) {
			camera.transform_getr().location += glm::vec3(0.0f, 0.0f, 1.0f) * -10.0f * deltaTime;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_E]) {
			camera.transform_getr().location += glm::vec3(0.0f, 0.0f, 1.0f) * 10.0f * deltaTime;
		}

		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_UP]) {
			auto rotation = glm::mat4(1.0)
				* glm::rotate(-1.0f * deltaTime, camera.right_getr())
			;
			camera.right_getr() = glm::vec4(camera.right_getr(), 1.0) * rotation;
			camera.forward_getr() = glm::vec4(camera.forward_getr(), 1.0) * rotation;
			camera.up_getr() = glm::vec4(camera.up_getr(), 1.0) * rotation;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_DOWN]) {
			auto rotation = glm::mat4(1.0)
				* glm::rotate(1.0f * deltaTime, camera.right_getr())
			;
			camera.right_getr() = glm::vec4(camera.right_getr(), 1.0) * rotation;
			camera.forward_getr() = glm::vec4(camera.forward_getr(), 1.0) * rotation;
			camera.up_getr() = glm::vec4(camera.up_getr(), 1.0) * rotation;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LEFT]) {
			auto rotation = glm::mat4(1.0)
				* glm::rotate(-1.0f * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f))
			;
			camera.right_getr() = glm::vec4(camera.right_getr(), 1.0) * rotation;
			camera.forward_getr() = glm::vec4(camera.forward_getr(), 1.0) * rotation;
			camera.up_getr() = glm::vec4(camera.up_getr(), 1.0) * rotation;
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RIGHT]) {
			auto rotation = glm::mat4(1.0)
				* glm::rotate(1.0f * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f))
			;
			camera.right_getr() = glm::vec4(camera.right_getr(), 1.0) * rotation;
			camera.forward_getr() = glm::vec4(camera.forward_getr(), 1.0) * rotation;
			camera.up_getr() = glm::vec4(camera.up_getr(), 1.0) * rotation;
		}


		// ****** UPDATE RENDERING ****** //

		glUseProgram(raymarch_program);
		uint samples = 3;
		static int stop = 0;
		static float timer = 0;
		if (stop == 0) {
		repeat(i, samples) {
			glUniform1i(glGetUniformLocation(raymarch_program, "rng_seed"), random(1, 100));
			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			stop = 1;
		}
		}
		timer += deltaTime;
		if (timer > 3 and stop == 1) {
			glUniform1i(glGetUniformLocation(raymarch_program, "rng_seed"), random(1, 100));
			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			stop = 2;
		}

		glUseProgram(rtarget_program);
		glBindVertexArray(rtargetVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, raymarch_tex);

		glUniform1i(glGetUniformLocation(rtarget_program, "tex_output"), 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	});

	return 0;
}
