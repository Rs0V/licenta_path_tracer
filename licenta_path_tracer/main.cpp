#include "sstream"
#include "Window.hpp"
#include "gtx/transform.hpp"
#include <gtc/type_ptr.hpp>
#include "Camera.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "PointLight.hpp"
#include "Sphere.hpp"
#include "RMOStructs.hpp"
#include "Cube.hpp"
#include "FX.hpp"
#include "Parent.hpp"
#include "UI.hpp"
#include "MPrincipledBSDF.hpp"
#include "MVolumeScatter.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



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
void createSSBO(std::string buffer_name, int buffer_index, size_t buffer_size, GLenum usage = GL_DYNAMIC_DRAW) {
	GLuint structs_ssbo;
	glGenBuffers(1, &structs_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, structs_ssbo);
	buffers[buffer_name] = structs_ssbo;

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer_index, structs_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, nullptr, usage);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
template<typename T> void setSSBOData(std::string buffer_name, const std::vector<T> &data, int size, int offset = 0) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[buffer_name]);
	void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy((char*)ptr + offset, data.data(), size);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
template<typename T> void getSSBOData(std::string buffer_name, std::vector<T>& data, int size, int offset = 0) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[buffer_name]);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, (void*)data.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
template<typename RMO, typename OBJ, typename CAST = OBJ> void setSSBOStructData(std::string buffer_name, const std::vector<std::shared_ptr<OBJ>> &structs, int offset = 0) {
	std::vector<RMO> rmo_structs;
	for (uint i = 0; i < structs.size(); i++) {
		if constexpr (std::is_same<OBJ, CAST>::value) {
			rmo_structs.push_back({});
			(*rmo_structs.rbegin()) = *structs[i];
			continue;
		}
		auto deriv = std::dynamic_pointer_cast<CAST>(structs[i]);
		if (deriv) {
			rmo_structs.push_back({});
			(*rmo_structs.rbegin()) = *deriv;
		}
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[buffer_name]);
	void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy((char*)ptr + offset, rmo_structs.data(), rmo_structs.size() * sizeof(RMO));

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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


struct Sprite {
	GLuint id = -1;
	std::string path;
	glm::uvec2 size = { 0, 0 };
	uint channels = 0;

	Sprite(std::string path) {
		int width, height, channels;
		unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (!image) {
			std::cerr << "Failed to load image!" << std::endl;
		}

		GLuint tex;
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		}
		stbi_image_free(image);

		this->id = tex;
		this->path = path;
		this->size = { width, height };
		this->channels = channels;
	}
	~Sprite() {
		glDeleteTextures(1, &this->id);
		this->id = -1;
		this->path.clear();
		this->size = { 0, 0 };
		this->channels = 0;
	}
	operator bool() const {
		return this->id > -1;
	}
};



int main(int argc, char* argv[]) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(8259);
	//_CrtSetBreakAlloc(477);
	//_CrtSetBreakAlloc(476);

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

	std::string obj_select_compute;
	readFile("obj_select.comp", obj_select_compute);

	std::string vertex;
	readFile("rtarget.vert", vertex);

	std::string fragment;
	readFile("rtarget.frag", fragment);


	/*
	std::string sh_basic_shapes;
	readFile("basic_shapes.comp", sh_basic_shapes);

	std::string sh_lights;
	readFile("lights.comp", sh_lights);

	std::string sh_materials;
	readFile("materials.comp", sh_materials);

	std::string sh_utils;
	readFile("utils.comp", sh_utils);

	sh_basic_shapes.replace(
		sh_basic_shapes.begin() + sh_basic_shapes.rfind("\n#include"),
		sh_basic_shapes.begin() + sh_basic_shapes.find('\n', sh_basic_shapes.rfind("\n#include") + 1),
		sh_utils
	);
	raymarch_compute.replace(
		raymarch_compute.begin() + raymarch_compute.rfind("\n#include"),
		raymarch_compute.begin() + raymarch_compute.find('\n', raymarch_compute.rfind("\n#include") + 1),
		sh_lights
	);
	raymarch_compute.replace(
		raymarch_compute.begin() + raymarch_compute.rfind("\n#include"),
		raymarch_compute.begin() + raymarch_compute.find('\n', raymarch_compute.rfind("\n#include") + 1),
		sh_materials
	);
	raymarch_compute.replace(
		raymarch_compute.begin() + raymarch_compute.rfind("\n#include"),
		raymarch_compute.begin() + raymarch_compute.find('\n', raymarch_compute.rfind("\n#include") + 1),
		sh_basic_shapes
	);
	std::ofstream debug_out("debug.txt");
	debug_out << raymarch_compute;
	*/

	std::cout << glGetString(GL_VERSION) << std::endl;
	std::cout << glGetString(GL_VENDOR) << std::endl;
	std::cout << glGetString(GL_RENDERER) << std::endl;
	std::cout << std::endl;

	#define CHECK_EXTS 0

	#if CHECK_EXTS
	GLint NUMEXT = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &NUMEXT);
	for (size_t i = 0; i < NUMEXT; i++) {
		std::cout << glGetStringi(GL_EXTENSIONS, i) << std::endl;
	}
	std::cout << std::endl;
	#endif // CHECK_EXTS

	if (!glNamedStringARB) {
		std::cerr << "Cannot use ARB extensions due to incompatible 'GL_RENDERER'!" << std::endl;
		exit(-1);
	}


	// Read Shader Header files
	addGLSLHeaderToFileSystem("utils.comp");
	addGLSLHeaderToFileSystem("basic_shapes.comp");
	addGLSLHeaderToFileSystem("materials.comp");
	addGLSLHeaderToFileSystem("lights.comp");
	addGLSLHeaderToFileSystem("shader_debug.comp");


	// Compile Shaders
	GLuint raymarch_program = -1;
	GLuint denoiser_program = createComputeShaderProgram(denoiser_compute);
	GLuint obj_select_program = -1;
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
	Camera camera = Camera(Transform(
		{ 0.0f, 30.0f, -70.0f },
		{ 0.0f,  0.0f,   0.0f }
	));

	std::vector<std::shared_ptr<Object>> objects;

	std::vector<std::shared_ptr<Sphere>> spheres;
	std::vector<std::shared_ptr<Cube>> cubes;
	std::vector<std::shared_ptr<Cylinder>> cylinders;
	std::vector<std::shared_ptr<Cone>> cones;

	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<PointLight>> point_lights;

	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<MPrincipledBSDF>> principledBSDFs;
	std::vector<std::shared_ptr<MVolumeScatter>> volume_scatters;

	std::vector<std::shared_ptr<Component>> components;
	std::vector<std::shared_ptr<boolean::Boolean>> booleans;


	glm::mat4 proj = glm::perspectiveFovLH_ZO(glm::radians(60.0f), (float)window.width_get(), (float)window.height_get(), 0.1f, 1000.0f);


	#define SCENE 2

	#if SCENE == 0

	#pragma region Create Materials

	materials.emplace_back(std::make_shared<MPrincipledBSDF>(
		Color::white,
		0.0f,
		0.12f
	));
	materials.emplace_back(std::make_shared<MPrincipledBSDF>(
		Color{ { 0.9f, 0.2f, 0.1f } },
		0.0f,
		0.68f
	));
	materials.emplace_back(std::make_shared<MPrincipledBSDF>(
		Color{ { 0.3f, 0.9f, 0.1f } },
		0.0f,
		0.68f
	));
	materials.emplace_back(std::make_shared<MPrincipledBSDF>(
		Color::white,
		0.0f,
		0.24f,
		1.45f,
		0.5f,
		1.0f
	));
	

	materials.emplace_back(std::make_shared<MVolumeScatter>(
		Color::white,
		0.2f
	));

	#pragma endregion


	#pragma region Create Objects

	objects.emplace_back(std::make_shared<Sphere>(
		Transform{ { 0.0f, 10.0f, 0.0f } },
		materials[3],
		10.0f
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{},
		materials[0],
		glm::vec3{ 20.0f, 20.0f, 20.0f }
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{ { 0.0f, -0.05f, 0.0f } },
		materials[0],
		glm::vec3{ 60.0f, 0.1f, 60.0f }
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{ { 0.0f, 60.05f, 0.0f } },
		materials[0],
		glm::vec3{ 60.0f, 0.1f, 60.0f }
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{ { -30.05f, 30.0f, 0.0f } },
		materials[1],
		glm::vec3{ 0.1f, 60.0f, 60.0f }
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{ { 30.05f, 30.0f, 0.0f } },
		materials[2],
		glm::vec3{ 0.1f, 60.0f, 60.0f }
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{ { 0.0f, 30.0f, 30.05f } },
		materials[0],
		glm::vec3{ 60.0f, 60.0f, 0.1f }
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { 0.0f, 10.0f, 0.0f } },
		materials[0],
		8.0f,
		20.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{
			{ -30.0f, -1000.0f, 10.0f },
			{   0.0f,     0.0f,  0.0f },
			{   1.0f,     2.0f,  1.0f }
		},
		materials[0],
		4.0f,
		12.0f
	));
	objects.emplace_back(std::make_shared<Cone>(
		Transform{
			{  25.0f, -1000.0f, -5.0f },
			{ 180.0f,     0.0f,  0.0f },
			{   1.0f,     2.0f,  3.0f }
		},
		materials[0],
		4.0f,
		6.0f
	));

	#pragma endregion

	// Sort Objects in order: Spheres, Cubes, Cylinders, Cones
	// Can't sort interface pointers (crazy...)
	/*
	std::sort(objects.begin(), objects.end(), [](const auto &a, const auto &b) {
		uint akey = -1;
		uint bkey = -1;

		r_is(a, Sphere, *) {
			akey = 0;
		} r_end
		r_is(a, Cube, *) {
			akey = 1;
		} r_end
		r_is(a, Cylinder, *) {
			akey = 2;
		} r_end
		r_is(a, Cone, *) {
			akey = 3;
		} r_end

		r_is(b, Sphere, *) {
			bkey = 0;
		} r_end
		r_is(b, Cube, *) {
			bkey = 1;
		} r_end
		r_is(b, Cylinder, *) {
			bkey = 2;
		} r_end
		r_is(b, Cone, *) {
			bkey = 3;
		} r_end

		return akey < bkey;
	});
	*/

	#pragma region Create Lights

	lights.emplace_back(std::make_shared<PointLight>(
		Transform{ { -20.0f, 55.0f, 0.0f } },
		Color::white,
		400.0f
	));
	lights.emplace_back(std::make_shared<PointLight>(
		Transform{ { 20.0f, 55.0f, 0.0f } },
		Color::white,
		100.0f
	));

	#pragma endregion

	#elif SCENE == 1
	
	#pragma region Create Materials
	
	materials.emplace_back(std::make_shared<MPrincipledBSDF>(
		Color::white,
		0.0f,
		0.18f
	));
	materials.emplace_back(std::make_shared<MPrincipledBSDF>(
		Color::white,
		0.96f,
		0.08f
	));
	
	materials.emplace_back(std::make_shared<MVolumeScatter>(
		Color::white,
		0.2f
	));
	
	#pragma endregion
	
	#pragma region Create Objects
	
	objects.emplace_back(std::make_shared<Sphere>(
		Transform{ { 0.0f, 30.0f, 80.0f } },
		materials[1],
		15.0f
	));

	objects.emplace_back(std::make_shared<Cube>(
		Transform{ { 0.0f, 60.0f, 0.0f } },
		materials[0],
		glm::vec3{ 120.0f, 120.0f, 280.0f }
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{ { 0.0f, 60.0f, 0.0f } },
		materials[0],
		glm::vec3{ 100.0f, 100.0f, 260.0f }
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{ { 0.0f, 90.0f, 0.0f } },
		materials[0],
		glm::vec3{ 140.0f, 20.0f, 240.0f }
	));

	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { 0.0f, 120.0f, 80.0f } },
		materials[0],
		45.0f,
		40.0f
	));

	float pillar_rad = 6.0f;
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { -35.0f, 60.0f, -70.0f } },
		materials[0],
		pillar_rad,
		120.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { 35.0f, 60.0f, -70.0f } },
		materials[0],
		pillar_rad,
		120.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { -35.0f, 60.0f, -40.0f } },
		materials[0],
		pillar_rad,
		120.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { 35.0f, 60.0f, -40.0f } },
		materials[0],
		pillar_rad,
		120.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { -35.0f, 60.0f, -10.0f } },
		materials[0],
		pillar_rad,
		120.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { 35.0f, 60.0f, -10.0f } },
		materials[0],
		pillar_rad,
		120.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { -35.0f, 60.0f, 20.0f } },
		materials[0],
		pillar_rad,
		120.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ { 35.0f, 60.0f, 20.0f } },
		materials[0],
		pillar_rad,
		120.0f
	));

	objects.emplace_back(std::make_shared<Cone>(
		Transform{
			{  25.0f, -1000.0f, -5.0f },
			{ 180.0f,     0.0f,  0.0f },
			{   1.0f,     2.0f,  3.0f }
		},
		materials[0],
		4.0f,
		6.0f
	));
	
	#pragma endregion
	
	#pragma region Create Lights
	
	lights.emplace_back(std::make_shared<PointLight>(
		Transform{ { 0.0f, 800.0f, 80.0f } },
		Color({ 0.9f, 0.7f, 0.8f }),
		1.0f,
		15.0f
	));
	
	#pragma endregion

	#elif SCENE == 2

	#pragma region Create Materials

	materials.emplace_back(std::make_shared<MPrincipledBSDF>(
		Color::white,
		0.0f,
		0.12f
	));
	

	materials.emplace_back(std::make_shared<MVolumeScatter>(
		Color::white,
		0.2f
	));

	#pragma endregion

	#pragma region Create Objects

	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ {0.0f, 0.0f, -15.0f} },
		materials[0],
		10.0f,
		5.0f
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ {0.0f, 0.0f, 9.0f} },
		materials[0],
		7.0f,
		5.0f
	));





	objects.emplace_back(std::make_shared<Sphere>(
		Transform{ {0.0f, -10000.0f, 0.0f} },
		materials[0],
		1.0f
	));
	objects.emplace_back(std::make_shared<Cube>(
		Transform{ {0.0f, -10000.0f, 0.0f} },
		materials[0],
		glm::vec3{ 1.0f, 1.0f, 1.0f }
	));
	objects.emplace_back(std::make_shared<Cylinder>(
		Transform{ {0.0f, -10000.0f, 0.0f} },
		materials[0],
		1.0f,
		1.0f
	));
	objects.emplace_back(std::make_shared<Cone>(
		Transform{ {0.0f, -10000.0f, 0.0f} },
		materials[0],
		1.0f,
		1.0f
	));

	#pragma endregion

	#pragma region Create Lights

	lights.emplace_back(std::make_shared<PointLight>(
		Transform{ { 0.0f, 0.0f, 0.0f } },
		Color::white,
		1.0f
	));

	#pragma endregion

	#endif


	// Filter objects / lights / materials
	for (auto &obj : objects) {
		auto sphere = std::dynamic_pointer_cast<Sphere>(obj);
		if (sphere) {
			spheres.emplace_back(sphere);
		}

		auto cube = std::dynamic_pointer_cast<Cube>(obj);
		if (cube) {
			cubes.emplace_back(cube);
		}

		auto cylinder = std::dynamic_pointer_cast<Cylinder>(obj);
		if (cylinder) {
			cylinders.emplace_back(cylinder);
		}

		auto cone = std::dynamic_pointer_cast<Cone>(obj);
		if (cone) {
			cones.emplace_back(cone);
		}
	}
	for (auto &light : lights) {
		auto point_light = std::dynamic_pointer_cast<PointLight>(light);
		if (point_light) {
			point_lights.emplace_back(point_light);
		}
	}
	for (auto &material : materials) {
		auto bsdf = std::dynamic_pointer_cast<MPrincipledBSDF>(material);
		if (bsdf) {
			principledBSDFs.emplace_back(bsdf);
		}

		auto vs = std::dynamic_pointer_cast<MVolumeScatter>(material);
		if (vs) {
			volume_scatters.emplace_back(vs);
		}
	}


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
	uint default_max_samples = 64;
	uint max_samples = default_max_samples;
	int samples = max_samples;

	uint max_diffuse_bounces      = 4;
	uint max_glossy_bounces       = 4;
	uint max_transmissive_bounces = 1;

	uint diffuse_bounces      = max_diffuse_bounces;
	uint glossy_bounces       = max_glossy_bounces;
	uint transmissive_bounces = max_transmissive_bounces;

	uint default_tile_size = 64;
	uint tile_size = default_tile_size;
	glm::uvec2 total_tiles = { glm::ceil((float)window.width_get() / tile_size), glm::ceil((float)window.height_get() / tile_size) };
	int tiles_remaining = total_tiles.x * total_tiles.y;

	auto reset_pathtracer = [&]() {
		tiles_remaining = total_tiles.x * total_tiles.y;
		samples = max_samples;

		/*
		int spheres = get_of_type<Sphere*>(objects).size() * sizeof(rmo::Sphere);
		int cubes = get_of_type<Cube*>(objects).size() * sizeof(rmo::Cube);
		int cylinders = get_of_type<Cylinder*>(objects).size() * sizeof(rmo::Cylinder);
		int cones = get_of_type<Cone*>(objects).size() * sizeof(rmo::Cone);

		int booleans = get_of_type<boolean::Boolean*>(components).size() * sizeof(rmo::CBoolean);

		int point_lights = get_of_type<PointLight*>(lights).size() * sizeof(rmo::PointLight);

		int principled_bsdfs = get_of_type<MPrincipledBSDF*>(materials).size() * sizeof(rmo::MPrincipledBSDF);
		int volume_scatters = get_of_type<MVolumeScatter*>(materials).size() * sizeof(rmo::MVolumeScatter);


		setSSBOData<Object, Sphere, rmo::Sphere>("BasicShapes", objects, 0);
		setSSBOData<Object, Cube, rmo::Cube>("BasicShapes", objects, spheres);
		setSSBOData<Object, Cylinder, rmo::Cylinder>("BasicShapes", objects, spheres + cubes);
		setSSBOData<Object, Cone, rmo::Cone>("BasicShapes", objects, spheres + cubes + cylinders);

		setSSBOData<Component, boolean::Boolean, rmo::CBoolean>("BasicShapes", components, spheres + cubes + cylinders + cones);

		setSSBOData<Light, PointLight, rmo::PointLight>("Props", lights, 0);

		setSSBOData<Material, MPrincipledBSDF, rmo::MPrincipledBSDF>("Props", materials, point_lights);
		setSSBOData<Material, MVolumeScatter, rmo::MVolumeScatter>("Props", materials, point_lights + principled_bsdfs);
		*/
	};

	int selected_object = 0;
	bool hoveringUI = false;
	glm::vec3 location = objects[selected_object]->transform_getrc().location;
	glm::vec3 rotation = objects[selected_object]->transform_getrc().rotation;
	glm::vec3 scale = objects[selected_object]->transform_getrc().scale;


	// Application Start function
	window.Start([&]() {
		glClearColor(0.5f, 0.1f, 0.2f, 1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		// Add Components to Objects
		#if SCENE == 0

		objects[1]->visible_set(false);
		objects[7]->visible_set(false);
		components.emplace_back(std::make_shared<boolean::Boolean>(objects[9], objects[8], boolean::Type::Union));

		#elif SCENE == 1

		objects[objects.size() - 1]->visible_set(false);

		objects[2]->visible_set(false);
		components.emplace_back(std::make_shared<boolean::Boolean>(objects[1], objects[2], boolean::Type::Difference, 0.01f));

		objects[3]->visible_set(false);
		components.emplace_back(std::make_shared<boolean::Boolean>(objects[1], objects[3], boolean::Type::Difference, 0.01f));

		objects[4]->visible_set(false);
		components.emplace_back(std::make_shared<boolean::Boolean>(objects[1], objects[4], boolean::Type::Difference, 0.01f));

		#elif SCENE == 2

		components.emplace_back(std::make_shared<boolean::Boolean>(objects[0], objects[1], boolean::Type::Union, 20.0f));

		#endif


		// Filter components
		for (auto &component : components) {
			auto boolean = std::dynamic_pointer_cast<boolean::Boolean>(component);
			if (boolean) {
				booleans.emplace_back(boolean);
			}
		}


		#pragma region Send Object Data to Ray-Marching Shader

		int spheres_bytes = spheres.size() * sizeof(rmo::Sphere);
		int cubes_bytes = cubes.size() * sizeof(rmo::Cube);
		int cylinders_bytes = cylinders.size() * sizeof(rmo::Cylinder);
		int cones_bytes = cones.size() * sizeof(rmo::Cone);
		int booleans_bytes = booleans.size() * sizeof(rmo::CBoolean);

		int point_lights_bytes = point_lights.size() * sizeof(rmo::PointLight);

		int bsdfs_bytes = principledBSDFs.size() * sizeof(rmo::MPrincipledBSDF);
		int vs_bytes = volume_scatters.size() * sizeof(rmo::MVolumeScatter);

		createSSBO("Screen", 0, sizeof(int) * window.width_get() * window.height_get(), GL_DYNAMIC_READ);
		createSSBO("BasicShapes", 1, 0
			+ spheres_bytes
			+ cubes_bytes
			+ cylinders_bytes
			+ cones_bytes
			+ booleans_bytes
		);
		createSSBO("Props", 2, 0
			+ point_lights_bytes
			+ bsdfs_bytes
			+ vs_bytes
		);

		std::vector<int> screen_data(window.width_get() * window.height_get(), -1);
		setSSBOData("Screen", screen_data, screen_data.size() * sizeof(int));

		setSSBOStructData<rmo::Sphere,   Sphere>(          "BasicShapes", spheres,   0);
		setSSBOStructData<rmo::Cube,     Cube>(            "BasicShapes", cubes,     spheres_bytes);
		setSSBOStructData<rmo::Cylinder, Cylinder>(        "BasicShapes", cylinders, spheres_bytes + cubes_bytes);
		setSSBOStructData<rmo::Cone,     Cone>(            "BasicShapes", cones,     spheres_bytes + cubes_bytes + cylinders_bytes);
		setSSBOStructData<rmo::CBoolean, boolean::Boolean>("BasicShapes", booleans,  spheres_bytes + cubes_bytes + cylinders_bytes + cones_bytes);

		setSSBOStructData<rmo::PointLight, PointLight>("Props", point_lights, 0);

		setSSBOStructData<rmo::MPrincipledBSDF, MPrincipledBSDF>("Props", principledBSDFs, point_lights_bytes);
		setSSBOStructData<rmo::MVolumeScatter,  MVolumeScatter>( "Props", volume_scatters, point_lights_bytes + bsdfs_bytes);

		// Set buffer sizes from SSBO
		auto set_buffer_size = [&](std::string& shader, int size) {
			size_t define_index = shader.find("#define");
			size_t space1 = shader.find(' ', define_index);
			size_t space2 = shader.find(' ', space1 + 1);

			shader.replace(
				shader.begin() + define_index,
				shader.begin() + shader.find('\n', define_index),
				"const int " + shader.substr(space1 + 1, space2 - space1 - 1) + " = " + std::to_string(size) + ";\n"
			);
		};

		set_buffer_size(raymarch_compute, spheres_bytes   / sizeof(rmo::Sphere));
		set_buffer_size(raymarch_compute, cubes_bytes     / sizeof(rmo::Cube));
		set_buffer_size(raymarch_compute, cylinders_bytes / sizeof(rmo::Cylinder));
		set_buffer_size(raymarch_compute, cones_bytes     / sizeof(rmo::Cone));
		set_buffer_size(raymarch_compute, booleans_bytes  / sizeof(rmo::CBoolean));

		set_buffer_size(raymarch_compute, point_lights_bytes / sizeof(rmo::PointLight));

		set_buffer_size(raymarch_compute, bsdfs_bytes / sizeof(rmo::MPrincipledBSDF));
		set_buffer_size(raymarch_compute, vs_bytes    / sizeof(rmo::MVolumeScatter));

		raymarch_program = createComputeShaderProgram(raymarch_compute);


		set_buffer_size(obj_select_compute, spheres_bytes   / sizeof(rmo::Sphere));
		set_buffer_size(obj_select_compute, cubes_bytes     / sizeof(rmo::Cube));
		set_buffer_size(obj_select_compute, cylinders_bytes / sizeof(rmo::Cylinder));
		set_buffer_size(obj_select_compute, cones_bytes     / sizeof(rmo::Cone));
		set_buffer_size(obj_select_compute, booleans_bytes  / sizeof(rmo::CBoolean));

		obj_select_program = createComputeShaderProgram(obj_select_compute);

		#pragma endregion


		// UI
		UI::uiGenFuncs.emplace_back([&]() {
			ImGuiIO& imguiIO = ImGui::GetIO(); (void)imguiIO;
			ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

			ImGui::Begin("Status Menu");

			static int selected_render_mode = 1;
			ImVec4 selected_tint(0.25f, 0.4f, 0.75f, 1.0f);
			bool need_to_pop = false;

			if (selected_render_mode == 0) {
				ImGui::PushStyleColor(ImGuiCol_Button, selected_tint);
				need_to_pop = true;
			}
			static Sprite solid_mode_icon("./solid_mode_icon.png");
			if (ImGui::ImageButton("solid_mode_button", solid_mode_icon.id, ImVec2(16, 16))) {
				diffuse_bounces      = 1;
				glossy_bounces       = 1;
				transmissive_bounces = 1;

				max_samples = 1;
				samples = max_samples;

				tile_size = glm::max(window.width_get(), window.height_get());
				total_tiles = { glm::ceil((float)window.width_get() / tile_size), glm::ceil((float)window.height_get() / tile_size) };
				tiles_remaining = total_tiles.x * total_tiles.y;

				selected_render_mode = 0;
			}
			if (need_to_pop) {
				ImGui::PopStyleColor();
				need_to_pop = false;
			}
			ImGui::SameLine();
			if (selected_render_mode == 1) {
				ImGui::PushStyleColor(ImGuiCol_Button, selected_tint);
				need_to_pop = true;
			}
			static Sprite render_mode_icon("./render_mode_icon.png");
			if (ImGui::ImageButton("render_mode_button", render_mode_icon.id, ImVec2(16, 16))) {
				diffuse_bounces      = max_diffuse_bounces;
				glossy_bounces       = max_glossy_bounces;
				transmissive_bounces = max_transmissive_bounces;

				max_samples = default_max_samples;
				samples = max_samples;

				tile_size = default_tile_size;
				total_tiles = { glm::ceil((float)window.width_get() / tile_size), glm::ceil((float)window.height_get() / tile_size) };
				tiles_remaining = total_tiles.x * total_tiles.y;

				selected_render_mode = 1;
			}
			if (need_to_pop) {
				ImGui::PopStyleColor();
				need_to_pop = false;
			}

			if (tiles_remaining > 0) {
				ImGui::Text("Rendering... %d/%d samples", std::min((int)max_samples, std::max(1, (int)max_samples - samples)), max_samples);
			}
			else {
				ImGui::Text("Rendering done!");
			}



			ImGui::Dummy(ImVec2(0, 18));
			ImGui::BeginChild("Camera");
			ImGui::Text("Camera:");

			static auto  cam_loc = camera.transform_getrc().location;
			static auto  cam_rot = camera.transform_getrc().rotation;
			static float cam_fov = 60.0f;

			ImGui::DragFloat3("Location", glm::value_ptr(cam_loc), 0.15f);
			ImGui::DragFloat3("Rotation", glm::value_ptr(cam_rot), 0.15f);
			ImGui::DragFloat("FOV", &cam_fov, 0.15f);

			if (ImGui::Button("Apply")) {
				camera.translate(cam_loc, 0);
				camera.rotate(cam_rot, 0);
				proj = glm::perspectiveFovLH_ZO(glm::radians(cam_fov), (float)window.width_get(), (float)window.height_get(), 0.1f, 1000.0f);

				reset_pathtracer();
			}

			ImGui::EndChild();

			ImGui::End();



			ImGui::Begin("Inspector Menu");

			int active_type = -1;
			if (selected_object - (int)spheres.size() - (int)cubes.size() - (int)cylinders.size() > -1) {
				ImGui::Text("Active Object:	Cone %d", selected_object);
				active_type = 3;
			}
			else if (selected_object - (int)spheres.size() - (int)cubes.size() > -1) {
				ImGui::Text("Active Object:	Cylinder %d", selected_object);
				active_type = 2;
			}
			else if (selected_object - (int)spheres.size() > -1) {
				ImGui::Text("Active Object:	Cube %d", selected_object);
				active_type = 1;
			}
			else {
				ImGui::Text("Active Object:	Sphere %d", selected_object);
				active_type = 0;
			}

			ImGui::Text("Index: ", selected_object);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(40.0f);
			if (ImGui::DragInt("##", &selected_object, 0.05f, 0, objects.size() - 1)) {
				location = objects[selected_object]->transform_getrc().location;
				rotation = objects[selected_object]->transform_getrc().rotation;
				scale = objects[selected_object]->transform_getrc().scale;
			}

			ImGui::BeginChild("Transform");

			ImGui::DragFloat3("Location", glm::value_ptr(location), 0.15f);
			ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.15f);
			ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.15f);

			if (ImGui::Button("Apply")) {
				objects[selected_object]->translate(location, 0);
				objects[selected_object]->rotate(rotation, 0);
				objects[selected_object]->scale(scale, 0);

				if (active_type == 0) {
					std::vector<std::shared_ptr<Sphere>> data = { spheres[selected_object] };
					setSSBOStructData<rmo::Sphere, Sphere>("BasicShapes", data, selected_object * sizeof(rmo::Sphere));
				}
				else if (active_type == 1) {
					std::vector<std::shared_ptr<Cube>> data = { cubes[selected_object - spheres.size()] };
					setSSBOStructData<rmo::Cube, Cube>("BasicShapes", data, spheres.size() * sizeof(rmo::Sphere) + (selected_object - spheres.size()) * sizeof(rmo::Cube));
				}
				else if (active_type == 2) {
					std::vector<std::shared_ptr<Cylinder>> data = { cylinders[selected_object - spheres.size() - cubes.size()] };
					setSSBOStructData<rmo::Cylinder, Cylinder>("BasicShapes", data, spheres.size() * sizeof(rmo::Sphere) + cubes.size() * sizeof(rmo::Cube) + (selected_object - spheres.size() - cubes.size()) * sizeof(rmo::Cylinder));
				}
				else if (active_type == 3) {
					std::vector<std::shared_ptr<Cone>> data = { cones[selected_object - spheres.size() - cubes.size() - cylinders.size()] };
					setSSBOStructData<rmo::Cone, Cone>("BasicShapes", data, spheres.size() * sizeof(rmo::Sphere) + cubes.size() * sizeof(rmo::Cube) + cylinders.size() * sizeof(rmo::Cylinder) + (selected_object - spheres.size() - cubes.size() - cylinders.size()) * sizeof(rmo::Cone));
				}

				reset_pathtracer();
			}

			ImGui::SameLine();
			if (tiles_remaining > 0) {
				if (ImGui::Button("Cancel")) {
					tiles_remaining = 0;
					samples = 0;
				}
			}
			else {
				if (ImGui::Button("Render")) {
					reset_pathtracer();
				}
			}

			ImGui::EndChild();

			ImGui::End();


			hoveringUI = ImGui::IsAnyItemFocused() or ImGui::IsAnyItemHovered() or ImGui::IsAnyItemActive();

			/*
			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / imguiIO.Framerate, imguiIO.Framerate);
			ImGui::End();
			*/
		});

	});
	// Application Update function
	window.Update([&](float deltaTime) {

		// Update App Title with current FPS
		SDL_SetWindowTitle(window.window_get(), std::to_string(1.0f / deltaTime).c_str());


		// ****** UPDATE LOGIC ****** //

		#pragma region Input

		// Setup Camera Input
		/*
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
		*/

		float speed = 25.0f;
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_W]) {
			camera.translate({ 0.0f, 0.0f, speed * deltaTime }, 2);
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_S]) {
			camera.translate({ 0.0f, 0.0f, -speed * deltaTime }, 2);
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_A]) {
			camera.translate({ -speed * deltaTime, 0.0f, 0.0f }, 2);
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_D]) {
			camera.translate({ speed * deltaTime, 0.0f, 0.0f }, 2);
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_Q]) {
			camera.translate({ 0.0f, -speed * deltaTime, 0.0f });
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_E]) {
			camera.translate({ 0.0f, speed * deltaTime, 0.0f });
			reset_pathtracer();
		}

		float sensitivity = 30.0f;
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_UP]) {
			camera.rotate({ -sensitivity * deltaTime * 2.0, 0.0f, 0.0f }, 2);
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_DOWN]) {
			camera.rotate({ sensitivity * deltaTime * 2.0, 0.0f, 0.0f }, 2);
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LEFT]) {
			camera.rotate({ 0.0f, sensitivity * deltaTime, 0.0f });
			reset_pathtracer();
		}
		if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RIGHT]) {
			camera.rotate({ 0.0f, -sensitivity * deltaTime, 0.0f });
			reset_pathtracer();
		}

		/*
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
		*/

		#pragma endregion

		glm::ivec2 mouse = { -1, -1 };
		if (!hoveringUI and SDL_GetMouseState(&mouse.x, &mouse.y) == SDL_BUTTON(1)) {
			std::vector<int> select_mask(window.width_get() * window.height_get());
			getSSBOData("Screen", select_mask, sizeof(int) * window.width_get() * window.height_get());

			const int &mouse_pixel = select_mask[mouse.y * window.width_get() + mouse.x];
			if (mouse_pixel > -1 and mouse_pixel < objects.size()) {
				selected_object = mouse_pixel;
				location = objects[selected_object]->transform_getrc().location;
				rotation = objects[selected_object]->transform_getrc().rotation;
				scale    = objects[selected_object]->transform_getrc().scale;
			}
		}


		// ****** UPDATE RENDERING ****** //

		// Object Select Buffer & Clear Raymarch Buffer
		if (tiles_remaining == total_tiles.x * total_tiles.y and samples == max_samples) {

			// Render Object Selection Buffer
			glUseProgram(obj_select_program);

			glm::mat4 view = glm::lookAtLH(
				camera.transform_getr().location,
				camera.transform_getr().location + camera.forward_getr(),
				camera.up_getr()
			);

			glUniformMatrix4fv(glGetUniformLocation(obj_select_program, "camera_proj"), 1, GL_FALSE, glm::value_ptr(proj));
			glUniformMatrix4fv(glGetUniformLocation(obj_select_program, "camera_view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniform3f(glGetUniformLocation(obj_select_program, "camera_loc"),
				camera.transform_getr().location.x,
				camera.transform_getr().location.y,
				camera.transform_getr().location.z
			);
			glUniform2i(glGetUniformLocation(obj_select_program, "screen_size"),
				window.width_get(),
				window.height_get()
			);

			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


			// Clear Raymarch Buffer
			glUseProgram(raymarch_program);

			glUniform1i(glGetUniformLocation(raymarch_program, "scene_change"), 1);
			glm::uvec2 tile_offset = { 0, 0 };
			glUniform2uiv(glGetUniformLocation(raymarch_program, "tile_offset"), 1, glm::value_ptr(tile_offset));

			glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			glUniform1i(glGetUniformLocation(raymarch_program, "scene_change"), 0);
		}

		// Actual Rendering
		if (tiles_remaining > 0) {

			// Render Tile Calculations
			uint tile_index = total_tiles.x * total_tiles.y - tiles_remaining;

			uint last_tile_size_x = window.width_get()  - (total_tiles.x - 1) * tile_size;
			uint last_tile_size_y = window.height_get() - (total_tiles.y - 1) * tile_size;

			uint tile_size_x = tile_index % total_tiles.x < total_tiles.x - 1 ? tile_size : last_tile_size_x;
			uint tile_size_y = tile_index / total_tiles.x < total_tiles.y - 1 ? tile_size : last_tile_size_y;

			uint tile_offset_x = tile_size * (tile_index % total_tiles.x);
			uint tile_offset_y = tile_size * (tile_index / total_tiles.x);

			glm::uvec2 tile_offset = { tile_offset_x, tile_offset_y };



			// Render Raymarch Sample
			if (samples > 0) {
				glUseProgram(raymarch_program);

				glUniform1f(glGetUniformLocation(raymarch_program, "random_f01"), random(0.0f, 1.0f));
				glUniform1i(glGetUniformLocation(raymarch_program, "samples"), max_samples);

				glm::mat4 view = glm::lookAtLH(
					camera.transform_getr().location,
					camera.transform_getr().location + camera.forward_getr(),
					camera.up_getr()
				);
				glUniformMatrix4fv(glGetUniformLocation(raymarch_program, "camera_proj"), 1, GL_FALSE, glm::value_ptr(proj));
				glUniformMatrix4fv(glGetUniformLocation(raymarch_program, "camera_view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniform3f(glGetUniformLocation(raymarch_program, "camera_loc"),
					camera.transform_getr().location.x,
					camera.transform_getr().location.y,
					camera.transform_getr().location.z
				);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, blueNoiseTex);
				blueNoiseTex = (blueNoiseTex == blueNoiseTex01) ? blueNoiseTex02 : blueNoiseTex01;

				glUniform2uiv(glGetUniformLocation(raymarch_program, "tile_offset"), 1, glm::value_ptr(tile_offset));

				glUniform1i(glGetUniformLocation(raymarch_program, "max_diffuse_bounces"), (int)diffuse_bounces);
				glUniform1i(glGetUniformLocation(raymarch_program, "max_glossy_bounces"), (int)glossy_bounces);
				glUniform1i(glGetUniformLocation(raymarch_program, "max_transmissive_bounces"), (int)transmissive_bounces);

				glDispatchCompute(tile_size_x / 16, tile_size_y / 16, 1);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



				// Denoise
				constexpr uint denoisingPasses = 1;

				glUseProgram(denoiser_program);

				glUniform1i(glGetUniformLocation(denoiser_program, "samples"), (int)max_samples - samples + 1);
				glUniform1i(glGetUniformLocation(denoiser_program, "max_samples"), (int)max_samples);
				glUniform2uiv(glGetUniformLocation(denoiser_program, "tile_offset"), 1, glm::value_ptr(tile_offset));

				for (uint i = 0; i < denoisingPasses; i++) {
					glDispatchCompute(tile_size_x / 16, tile_size_y / 16, 1);
					glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				}
				/*
				// Denoiser Blur-Pass
				glUniform1i(glGetUniformLocation(denoiser_program, "blur"), 0);
				glDispatchCompute(window.width_get() / 16, window.height_get() / 16, 1);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				glUniform1i(glGetUniformLocation(denoiser_program, "blur"), 1);
				*/



				samples--;
			}
			if (samples == 0) {
				tiles_remaining--;
				samples = max_samples;
			}
		}

		// Render final Output to Screen
		glUseProgram(rtarget_program);
		glBindVertexArray(rtargetVAO);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, output_tex);

		glUniform1i(glGetUniformLocation(rtarget_program, "tex_output"), 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	});

	return 0;
}
