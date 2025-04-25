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
template<typename OBJ, typename CAST, typename RMO> void setSSBOStructData(std::string buffer_name, const std::vector<OBJ*> &structs, int offset = 0) {
	auto casts = get_of_type<CAST*>(structs);
	std::vector<RMO> rmo_structs(casts.size());
	for (uint i = 0; i < rmo_structs.size(); i++) {
		rmo_structs[i] = *casts[i];
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[buffer_name]);
	void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy((char*)ptr + offset, rmo_structs.data(), sizeof(RMO) * rmo_structs.size());

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
	

	// Read Shader Header files
	addGLSLHeaderToFileSystem("utils.comp");
	addGLSLHeaderToFileSystem("basic_shapes.comp");
	addGLSLHeaderToFileSystem("materials.comp");
	addGLSLHeaderToFileSystem("lights.comp");
	addGLSLHeaderToFileSystem("shader_debug.comp");
	

	// Compile Shaders
	GLuint raymarch_program   = -1;
	GLuint denoiser_program   = createComputeShaderProgram(denoiser_compute);
	GLuint obj_select_program = -1;
	GLuint rtarget_program    = createShaderProgram(vertex, fragment);

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
	std::vector<Object*> objects;
	std::vector<Light*> lights;
	std::vector<Material*> materials;
	std::vector<Component*> components;

	glm::mat4 proj = glm::perspectiveFovLH_ZO(glm::radians(60.0f), (float)window.width_get(), (float)window.height_get(), 0.1f, 1000.0f);


	#pragma region Create Materials

	materials.emplace_back(new MPrincipledBSDF(
		Color::white * 0.8f,
		0.0f,
		0.18f
	));
	materials.emplace_back(new MPrincipledBSDF(
		Color({ 0.9f, 0.2f, 0.1f }),
		0.0f,
		0.78f
	));
	materials.emplace_back(new MPrincipledBSDF(
		Color({ 0.3f, 0.9f, 0.1f }),
		0.0f,
		0.78f
	));
	materials.emplace_back(new MPrincipledBSDF(
		Color::white * 0.4f,
		1.0f,
		0.18f
	));
	

	materials.emplace_back(new MVolumeScatter(
		Color::white,
		0.2f
	));

	#pragma endregion


	#pragma region Create Objects

	objects.emplace_back(new Sphere(
		{
			{ 0.0f, 10.0f, 0.0f },
			{ 0.0f,  0.0f, 0.0f },
			{ 1.0f,  1.0f, 1.0f }
		},
		materials[3],
		10.0f
	));
	objects.emplace_back(new Cube(
		{
			{ 0.0f, 0.0f, 0.0f }
		},
		materials[0],
		{ 20.0f, 20.0f, 20.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ 0.0f, -0.05f, 0.0f }
		},
		materials[0],
		{ 60.0f, 0.1f, 60.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ 0.0f, 60.05f, 0.0f }
		},
		materials[0],
		{ 60.0f, 0.1f, 60.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ -30.05f, 30.0f, 0.0f }
		},
		materials[1],
		{ 0.1f, 60.0f, 60.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ 30.05f, 30.0f, 0.0f }
		},
		materials[2],
		{ 0.1f, 60.0f, 60.0f }
	));
	objects.emplace_back(new Cube(
		{
			{ 0.0f, 30.0f, 30.05f }
		},
		materials[0],
		{ 60.0f, 60.0f, 0.1f }
	));
	objects.emplace_back(new Cylinder(
		{
			{ 0.0f, 10.0f, 0.0f },
			{ 0.0f,  0.0f, 0.0f },
			{ 1.0f,  1.0f, 1.0f }
		},
		materials[0],
		8.0f,
		20.0f
	));
	objects.emplace_back(new Cylinder(
		{
			{ -30.0f, -1000.0f, 10.0f },
			{   0.0f,     0.0f,  0.0f },
			{   1.0f,     2.0f,  1.0f }
		},
		materials[0],
		4.0f,
		12.0f
	));
	objects.emplace_back(new Cone(
		{
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

	lights.emplace_back(new PointLight(
		{
			{ -20.0f, 55.0f, 0.0f }
		},
		Color::white,
		40.0f
	));
	lights.emplace_back(new PointLight(
		{
			{ 20.0f, 55.0f, 0.0f }
		},
		Color::white,
		40.0f
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
	constexpr uint max_samples = 64;
	int samples = max_samples;

	constexpr uint tile_size = 128;
	const glm::uvec2 total_tiles = { glm::ceil((float)window.width_get() / tile_size), glm::ceil((float)window.height_get() / tile_size) };
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
		//components.emplace_back(new boolean::Boolean(objects[0], objects[1], boolean::Type::Difference));
		//components.emplace_back(new boolean::Boolean(objects[0], objects[7], boolean::Type::Intersect));
		objects[1]->visible_set(false);
		objects[7]->visible_set(false);

		components.emplace_back(new boolean::Boolean(objects[9], objects[8], boolean::Type::Union));

		//objects[0]->components_getr().emplace_back(new Parent(objects[0], objects[1]));

		//objects[1]->rotate({ -60.0f, -20.0f, 0.0f });
		//dynamic_cast<Parent*>(objects[0]->components_getrc()[0])->applyTransform();


		#pragma region Send Object Data to Ray-Marching Shader

		int spheres = get_of_type<Sphere*>(objects).size() * sizeof(rmo::Sphere);
		int cubes = get_of_type<Cube*>(objects).size() * sizeof(rmo::Cube);
		int cylinders = get_of_type<Cylinder*>(objects).size() * sizeof(rmo::Cylinder);
		int cones = get_of_type<Cone*>(objects).size() * sizeof(rmo::Cone);

		int booleans = get_of_type<boolean::Boolean*>(components).size() * sizeof(rmo::CBoolean);

		int point_lights = get_of_type<PointLight*>(lights).size() * sizeof(rmo::PointLight);

		int principled_bsdfs = get_of_type<MPrincipledBSDF*>(materials).size() * sizeof(rmo::MPrincipledBSDF);
		int volume_scatters = get_of_type<MVolumeScatter*>(materials).size() * sizeof(rmo::MVolumeScatter);


		createSSBO("Screen", 0, sizeof(int) * window.width_get() * window.height_get(), GL_DYNAMIC_READ);
		createSSBO("BasicShapes", 1, spheres + cubes + cylinders + cones + booleans);
		createSSBO("Props", 2, point_lights + principled_bsdfs + volume_scatters);

		std::vector<int> screen_data(window.width_get() * window.height_get(), -1);
		setSSBOData("Screen", screen_data, sizeof(int) * screen_data.size());

		setSSBOStructData<Object, Sphere, rmo::Sphere>("BasicShapes", objects, 0);
		setSSBOStructData<Object, Cube, rmo::Cube>("BasicShapes", objects, spheres);
		setSSBOStructData<Object, Cylinder, rmo::Cylinder>("BasicShapes", objects, spheres + cubes);
		setSSBOStructData<Object, Cone, rmo::Cone>("BasicShapes", objects, spheres + cubes + cylinders);

		setSSBOStructData<Component, boolean::Boolean, rmo::CBoolean>("BasicShapes", components, spheres + cubes + cylinders + cones);

		setSSBOStructData<Light, PointLight, rmo::PointLight>("Props", lights, 0);

		setSSBOStructData<Material, MPrincipledBSDF, rmo::MPrincipledBSDF>("Props", materials, point_lights);
		setSSBOStructData<Material, MVolumeScatter, rmo::MVolumeScatter>("Props", materials, point_lights + principled_bsdfs);


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

		set_buffer_size(raymarch_compute, spheres / sizeof(rmo::Sphere));
		set_buffer_size(raymarch_compute, cubes / sizeof(rmo::Cube));
		set_buffer_size(raymarch_compute, cylinders / sizeof(rmo::Cylinder));
		set_buffer_size(raymarch_compute, cones / sizeof(rmo::Cone));

		set_buffer_size(raymarch_compute, booleans / sizeof(rmo::CBoolean));

		set_buffer_size(raymarch_compute, point_lights / sizeof(rmo::PointLight));

		set_buffer_size(raymarch_compute, principled_bsdfs / sizeof(rmo::MPrincipledBSDF));
		set_buffer_size(raymarch_compute, volume_scatters / sizeof(rmo::MVolumeScatter));

		raymarch_program = createComputeShaderProgram(raymarch_compute);


		set_buffer_size(obj_select_compute, spheres / sizeof(rmo::Sphere));
		set_buffer_size(obj_select_compute, cubes / sizeof(rmo::Cube));
		set_buffer_size(obj_select_compute, cylinders / sizeof(rmo::Cylinder));
		set_buffer_size(obj_select_compute, cones / sizeof(rmo::Cone));

		set_buffer_size(obj_select_compute, booleans / sizeof(rmo::CBoolean));

		obj_select_program = createComputeShaderProgram(obj_select_compute);

		#pragma endregion


		// UI
		UI::uiGenFuncs.emplace_back([&]() {
			ImGuiIO& imguiIO = ImGui::GetIO(); (void)imguiIO;
			ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

			ImGui::Begin("Status Menu");

			if (tiles_remaining > 0) {
				ImGui::Text("Rendering... %d/%d samples", std::min((int)max_samples, std::max(1, (int)max_samples - samples)), max_samples);
			}
			else {
				ImGui::Text("Rendering done!");
			}

			ImGui::End();


			ImGui::Begin("Inspector Menu");

			if (dynamic_cast<Sphere*>(objects[selected_object]) != nullptr) {
				ImGui::Text("Active Object:	Sphere %d", selected_object);
			}
			else if (dynamic_cast<Cube*>(objects[selected_object]) != nullptr) {
				ImGui::Text("Active Object:	Cube %d", selected_object);
			}
			else if (dynamic_cast<Cylinder*>(objects[selected_object]) != nullptr) {
				ImGui::Text("Active Object:	Cylinder %d", selected_object);
			}
			else if (dynamic_cast<Cone*>(objects[selected_object]) != nullptr) {
				ImGui::Text("Active Object:	Cone %d", selected_object);
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

				std::vector<Object*> data = { objects[selected_object] };
				if (dynamic_cast<Sphere*>(objects[selected_object]) != nullptr) {
					setSSBOStructData<Object, Sphere, rmo::Sphere>("BasicShapes", data, sizeof(rmo::Sphere) * selected_object);
				}
				else if (dynamic_cast<Cube*>(objects[selected_object]) != nullptr) {
					int spheres = get_of_type<Sphere*>(objects).size();
					setSSBOStructData<Object, Cube, rmo::Cube>("BasicShapes", data, sizeof(rmo::Sphere) * spheres + sizeof(rmo::Cube) * (selected_object - spheres));
				}
				else if (dynamic_cast<Cylinder*>(objects[selected_object]) != nullptr) {
					int spheres = get_of_type<Sphere*>(objects).size();
					int cubes = get_of_type<Cube*>(objects).size();
					setSSBOStructData<Object, Cylinder, rmo::Cylinder>("BasicShapes", data, sizeof(rmo::Sphere) * spheres + sizeof(rmo::Cube) * cubes + sizeof(rmo::Cylinder) * (selected_object - spheres - cubes));
				}
				else if (dynamic_cast<Cone*>(objects[selected_object]) != nullptr) {
					int spheres = get_of_type<Sphere*>(objects).size();
					int cubes = get_of_type<Cube*>(objects).size();
					int cylinders = get_of_type<Cylinder*>(objects).size();
					setSSBOStructData<Object, Cone, rmo::Cone>("BasicShapes", data, sizeof(rmo::Sphere) * spheres + sizeof(rmo::Cube) * cubes + sizeof(rmo::Cylinder) * cylinders + sizeof(rmo::Cone) * (selected_object - spheres - cubes - cylinders));
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
		float speed = 25.0f;
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

				glDispatchCompute(tile_size_x / 16, tile_size_y / 16, 1);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



				// Denoise
				constexpr uint denoisingPasses = 4;

				glUseProgram(denoiser_program);

				glUniform1i(glGetUniformLocation(denoiser_program, "samples"), (int)max_samples - samples + 1);
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
