#pragma once
#include "Utilities.hpp"


namespace filters {
	template<typename T> using mat = std::vector<std::vector<T>>;

	static mat<float> createGaussianKernel(int kernelSize, float sigma) {
		mat<float> kernel(kernelSize, std::vector<float>(kernelSize, 0.0f));
		float sum = 0.0f;
		int center = kernelSize / 2;

		for (int y = 0; y < kernelSize; y++) {
			for (int x = 0; x < kernelSize; x++) {
				float dx = x - center;
				float dy = y - center;
				kernel[y][x] = std::exp(-(dx * dx + dy * dy) / (2 * sigma * sigma));
				sum += kernel[y][x];
			}
		}

		for (int y = 0; y < kernelSize; y++) {
			for (int x = 0; x < kernelSize; x++) {
				kernel[y][x] /= sum;
			}
		}

		return kernel;
	}

	static mat<float> applyGaussianBlur(const mat<float> &image, int kernelSize, float sigma) {
		mat<float> kernel = createGaussianKernel(kernelSize, sigma);
		int rows = image.size();
		int cols = image[0].size();
		mat<float> output(rows, std::vector<float>(cols, 0.0f));
		int offset = kernelSize / 2;

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				float sum = 0.0f;

				for (int ky = -offset; ky <= offset; ky++) {
					for (int kx = -offset; kx <= offset; kx++) {
						int ni = i + ky;
						int nj = j + kx;

						if (ni >= 0
						and ni < rows
						and nj >= 0
						and nj < cols) {
							sum += image[ni][nj] * kernel[ky + offset][kx + offset];
						}
					}
				}

				output[i][j] = sum;
			}
		}

		return output;
	}

	static mat<float> applyHighPassFilter(const mat<float>& image) {
		mat<float> lowPass = applyGaussianBlur(image, 15, 2.0);

		mat<float> output = image;
		float min = 999999999, max = -999999999;
		for (int y = 0; y < output.size(); y++) {
			for (int x = 0; x < output[0].size(); x++) {
				output[y][x] -= lowPass[y][x];
				if (output[y][x] < min) {
					min = output[y][x];
				}
				if (output[y][x] > max) {
					max = output[y][x];
				}
			}
		}

		// Normalize values in output
		for (int y = 0; y < output.size(); y++) {
			for (int x = 0; x < output[0].size(); x++) {
				output[y][x] -= min;
				output[y][x] /= max - min;
			}
		}

		return output;
	}
}


namespace generators {
	template<typename T> using mat = std::vector<std::vector<T>>;

	static mat<float> generateWhiteNoise(int width, int height) {
		auto [dist, gen] = random_gen(0.0f, 1.0f);
		mat<float> texture(height, std::vector<float>(width, 0.0f));

		for (int y = 0; y < texture.size(); y++) {
			for (int x = 0; x < texture[0].size(); x++) {
				texture[y][x] = dist(gen);
			}
		}

		return texture;
	}

	static mat<float> generateBlueNoise(int width, int height) {
		mat<float> whiteNoise = generateWhiteNoise(width, height);
		mat<float> blueNoise = filters::applyHighPassFilter(whiteNoise);
		return blueNoise;
	}

	static mat<glm::vec4> valuesToGrayscale(const mat<float>& image) {
		mat<glm::vec4> grayscale(image.size(), std::vector<glm::vec4>(image[0].size(), glm::vec4(0)));

		for (int y = 0; y < image.size(); y++) {
			for (int x = 0; x < image[0].size(); x++) {
				grayscale[y][x] = glm::vec4(image[y][x]);
				grayscale[y][x].a = 1.0f;
			}
		}

		return grayscale;
	}
}
