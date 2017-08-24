#pragma once
#include <freeglut.h>
#include <fstream>
#include <complex.h>
#include <future>
#include <thread>
#include <amp.h>
#include <iomanip>
#include <codecvt>
#include "dependencies.h"
#include "quad.h"
#include "input.h"
#include "Camera.h"
#include "FreeCamera.h"

#define TILE_SIZE 8
// The size of the image to generate.
#define WIDTH 2048  
#define HEIGHT 2048
#define DATA_SIZE (HEIGHT * WIDTH)

using namespace concurrency;

enum CALC_MANDELBROT
{
	AMP_MANDELBROT,
	AMP_PIXEL_MANDELBROT,
	AMP_BARRIER_MANDELBROT,
};

enum
{
	NVIDIA,
	MICROSOFT_BASIC_RENDER_DRIVER,
	SOFTWARE_ADAPTER,
	CPU_ACCELERATOR
};

class Mandelbrot
{
public:
	Mandelbrot(Input * in);
	~Mandelbrot();
	// Mandlebrot OpenGL function calls
	void update(float dt);
	void render();
private:
	// Classes pointers
	Input * input;
	Camera * camera;
	FreeCamera freeCamera;
	// initialize funciton
	void init(Input * in);
	// displaying texture variables
	Vector3 scale_;
	Vector3 translate_;
	Vector3 zoom_;
	float zoom_scale_;
	// enum to call specific Mandelbrot funstions
	CALC_MANDELBROT calc_mandelbrot_;
	// accelerators variables
	std::vector<accelerator> accls_;
	uint8_t current_accelerator_;
	// variables passed to lambda functions of Mandelbrot calcualtion functions
	unsigned long max_iterations_; // The number of times to iterate before we assume that a point isn't in the Mandelbrot set.
	unsigned b_, g_, r_;           // blue, green and red colours
	// Different methods of calculating mandelbrot
	void cpu_mandelbrot(float left, float right, float top, float bottom);
	void amp_mandelbrot(float left, float right, float top, float bottom);
	void amp_pixel_mandelbrot(float left, float right, float top, float bottom);
	void amp_barrier_mandelbrot(float left, float right, float top, float bottom);
	// amp_mandelbrot
	std::array<uint32_t, DATA_SIZE> image_amp_mandelbrot_;
	std::vector<uint8_t> pixel_amp_mandelbrot_;
	// amp_pixel_mandelbrot
	std::array<uint32_t, DATA_SIZE> image_amp_pixel_mandlebrot_;
	std::array<int, DATA_SIZE * 3> pixel_amp_pixel_mandlebrot_;
	// amp_barrier_mandelbrot
	std::array<uint32_t, DATA_SIZE> image_amp_barrier_mandelbrot_;
	std::vector<int> pixel_amp_barrier_mandelbrot_;
	// maximum timing
	int max_timings_;
	// current number of timings
	int i_;
	// 
	bool timing_;
	// 
	bool calculate_;
	// textures variables
	GLenum texture_;
	GLenum amp_mandelbrot_texture_;
	GLenum amp_pixel_mandelbrot_texture_;
	GLenum amp_barrier_mandelbrot_texture_;
	// flag for calling once a lambda function in update()
	std::once_flag flag_;
	std::once_flag accls_amp_mandelbrot_flag;
	std::once_flag accls_pixel_mandelbrot_flag;
	std::once_flag accls_barrier_mandelbrot_flag;
	// file to store timings
	std::ofstream file_amp_mandelbrot_nvidia_;
	std::ofstream file_amp_mandelbrot_msc_basic_render_driver_;
	std::ofstream file_amp_mandelbrot_software_adapter_;
	std::ofstream file_amp_mandelbrot_cpu_accelerator_;
	std::ofstream file_amp_pixel_mandelbrot_nvidia_;
	std::ofstream file_amp_pixel_mandelbrot_msc_basic_render_driver_;
	std::ofstream file_amp_pixel_mandelbrot_software_adapter_;
	std::ofstream file_amp_pixel_mandelbrot_cpu_accelerator_;
	std::ofstream file_amp_barrier_mandelbrot_nvidia_;
	std::ofstream file_amp_barrier_mandelbrot_msc_basic_render_driver_;
	std::ofstream file_amp_barrier_mandelbrot_software_adapter_;
	std::ofstream file_amp_barrier_mandelbrot_cpu_accelerator_;
	// convert std::wstring into std::string
	std::string ws2s(const std::wstring& wstr);
};



