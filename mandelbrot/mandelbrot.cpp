#include "mandelbrot.h"
#include "Complex.h"

Mandelbrot::Mandelbrot(Input * in)
{
	//OpenGL settings			
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);				// Really Nice Perspective Calculations
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// For a textured object we can control how the final RGB for the rendered pixel is set (combination of texture and geometry colours)
	glEnable(GL_TEXTURE_2D);										// Enable texturing
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);			// Set The Blending Function For Translucency
	//glEnable(GL_COLOR_MATERIAL);									// Without it all glColor3f() changes are ignored when lighting is enabled
	// initialize variables
	init(in);
}

Mandelbrot::~Mandelbrot()
{
	file_amp_mandelbrot_nvidia_.close();
	file_amp_mandelbrot_msc_basic_render_driver_.close();
	file_amp_mandelbrot_software_adapter_.close();
	file_amp_mandelbrot_cpu_accelerator_.close();
	file_amp_pixel_mandelbrot_nvidia_.close();
	file_amp_pixel_mandelbrot_msc_basic_render_driver_.close();
	file_amp_pixel_mandelbrot_software_adapter_.close();
	file_amp_pixel_mandelbrot_cpu_accelerator_.close();
	file_amp_barrier_mandelbrot_nvidia_.close();
	file_amp_barrier_mandelbrot_msc_basic_render_driver_.close();
	file_amp_barrier_mandelbrot_software_adapter_.close();
	file_amp_barrier_mandelbrot_cpu_accelerator_.close();
}

void Mandelbrot::init(Input * in)
{
	input = in;
	camera = &freeCamera;
	scale_.set(1.0f, 1.0f, 0.0f);
	translate_.set(0.0f, 0.0f, 0.0f);
	zoom_.set(1.0f, 1.0f, 0.0f);
	zoom_scale_ = 1.0f;
	// default mandelbrot calculation function
	calc_mandelbrot_ = AMP_MANDELBROT;
	// maximum number of iterations for all the Mandelbrot functions
	max_iterations_ = 0;
	// condition flag to calculate Mandlebrot only when the funciton is called
	calculate_ = false;
	// vector to hold all accelerators
	accls_ = accelerator::get_all();
	// varaibles to hold number of the current accelerator
	current_accelerator_ = 0;
	// 
	pixel_amp_mandelbrot_.reserve(DATA_SIZE * 3);
	pixel_amp_barrier_mandelbrot_ = std::vector<int>(DATA_SIZE * 3);
	// colours
	r_ = 250;
	g_ = 68;
	b_ = 32;
	// timing number of times variables
	i_ = 0;
	max_timings_ = 100;
	timing_ = false;
	// files to write timings to
	file_amp_mandelbrot_nvidia_.open("amp_mandelbrot_NVIDIA_.csv");
	file_amp_mandelbrot_msc_basic_render_driver_.open("amp_mandelbrot_MICROSOFT_basic_render_driver_.csv");
	file_amp_mandelbrot_software_adapter_.open("amp_mandelbrot_software_adapter_.csv");
	file_amp_mandelbrot_cpu_accelerator_.open("amp_mandelbrot_cpu_accelerator_.csv");
	file_amp_pixel_mandelbrot_nvidia_.open("amp_pixel_mandelbrot_NVIDIA_.csv");
	file_amp_pixel_mandelbrot_msc_basic_render_driver_.open("amp_pixel_mandelbrot_MICROSOFT_basic_render_driver_.csv");
	file_amp_pixel_mandelbrot_software_adapter_.open("amp_pixel_mandelbrot_software_adapter_.csv");
	file_amp_pixel_mandelbrot_cpu_accelerator_.open("amp_pixel_mandelbrot_cpu_accelerator_.csv");
	file_amp_barrier_mandelbrot_nvidia_.open("amp_barrier_mandelbrot_NVIDIA_.csv");
	file_amp_barrier_mandelbrot_msc_basic_render_driver_.open("amp_barrier_mandelbrot_MICROSOFT_basic_render_driver_.csv");
	file_amp_barrier_mandelbrot_software_adapter_.open("amp_barrier_mandelbrot_software_adapter_.csv");
	file_amp_barrier_mandelbrot_cpu_accelerator_.open("amp_barrier_mandelbrot_cpu_accelerator_.csv");
}

// convert wstring to string
std::string Mandelbrot::ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

// Render the Mandelbrot set into the image array.
// The parameters specify the region on the complex plane to plot.
void Mandelbrot::cpu_mandelbrot(float left, float right, float top, float bottom)
{
	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.

			complex<double> c(left + (x * (right - left) / WIDTH),
				top + (y * (bottom - top) / HEIGHT));

			// Start off z at (0, 0).
			complex<double> z(0.0, 0.0);

			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			int iterations = 0;
			while (abs(z) < 2.0 && iterations < max_iterations_)
			{
				z = (z * z) + c;

				++iterations;
			}

			uint8_t r, g, b;
			if (iterations == max_iterations_)
			{
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				//image[x*WIDTH + y] = 0x000000; // black
				r = 250;
				g = 32;
				b = 32;
			}
			else
			{
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				//image[x*WIDTH + y] = 0xFFFFFF; // white
				r = iterations * (iterations - 3) * 250;
				g = iterations * (iterations - 2) * 250;
				b = iterations * (iterations - 1) * 32;
			}
			//image[x*WIDTH + y] = (r << 16) | (g << 8) | (b);
			//// calculate pixel image
			//pixel.clear();
			//pixel.push_back(image[x*HEIGHT + y] & 0xFF); // blue channel
			//pixel.push_back((image[x*HEIGHT + y] >> 8) & 0xFF); // green channel
			//pixel.push_back((image[x*HEIGHT + y] >> 16) & 0xFF); // red channel
		}
	}
	i_++;
	calculate_ = false;
}

void Mandelbrot::amp_mandelbrot(float left, float right, float top, float bottom)
{
	/// Also observe that the parallel_for_each is not using member variables directly because that would involve
	/// marshaling the this pointer which is not allowed by one of the restrictions. 
	/// Hence the member variables are copied to local variables and then used inside the kernel.
	// extent - the extent class specifies the lenght of the data 
	// in each dimension of the array or array_view object
	// you can create an extent object and use it to create an array or array_view object
	// you can also specify	the extent using explicit parameters
	// in the array or array_view constructor
	// in this case an extent object is used to 
	// create a 2D array_view object 
	// with rows and columns defined 
	// by WIDTH and HEIGHT of the Mandelbrot set

	// call once to put into file name of the function, accelerator and number of tiles
	std::call_once(accls_amp_mandelbrot_flag, [=]() {
		switch (current_accelerator_) {
		case NVIDIA: {
			file_amp_mandelbrot_nvidia_ << "amp_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_mandelbrot_nvidia_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case MICROSOFT_BASIC_RENDER_DRIVER: {
			file_amp_mandelbrot_msc_basic_render_driver_ << "amp_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_mandelbrot_msc_basic_render_driver_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case SOFTWARE_ADAPTER: {
			file_amp_mandelbrot_software_adapter_ << "amp_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_mandelbrot_software_adapter_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case CPU_ACCELERATOR: {
			file_amp_mandelbrot_cpu_accelerator_ << "amp_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_mandelbrot_cpu_accelerator_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		}
	});

	// accelerator to be used with parallel for each
	//accelerator_view av1 = accelerator(accelerator::default_accelerator).default_view;
	accelerator_view av = accls_[current_accelerator_].default_view;

	// array view - wraper for image array to calculate mandelbrot
	image_amp_mandelbrot_.empty();
	extent<2> e(WIDTH, HEIGHT);
	array_view<uint32_t, 2> image_array_view(e, image_amp_mandelbrot_);
	image_array_view.discard_data(); // discarding image_array_view data and empting image array speeds up calculations

	// TODO delete this - array_view<uint32_t, 1> v = pixel_int_array.reinterpret_as<uint32_t>();
	// variables to pass to parallel_for_each lambda function
	unsigned max_iter = max_iterations_;
	unsigned r = r_;
	unsigned g = g_;
	unsigned b = b_;
	// a tile - a bunch/group/block of threads (a thread block/Direct Compute - a working group/OpenCL)
	// a tile - a group of threads within the thread block
	// tiling up to 3D
	try
	{
		// kernel - code that's embeded in parallel_for_each function   
		parallel_for_each(
			av,                                                   // what accelerator to use
			image_array_view.extent.tile<TILE_SIZE, TILE_SIZE>(), // times kernel is to tun - compute domain     
			[=]                                                   // pass data to computation tho' capture clause by value [=]
			(tiled_index<TILE_SIZE, TILE_SIZE> t_idx)             // index to access elem. of array_view
			mutable                                               // mutable allows copies to be modified, but not originals
			restrict(amp)                                         // subset of the C++ language that C++ AMP can accelerate is used
		{                                                                     
			// index - represents a unique point in N-dimensional space. 
			// The index Class specifies a location in the array or array_view object 
			// (by encapsulating the offset from the origin in each dimension into one object)
			// the first parameter in the index constructor gives row number,
			// and the second parameter gives column (within row) for 2D
			index<2> idx = t_idx.global; // changes for tiled index - (latency hiding?)

			// Start off z at (0, 0).

			Complex z = { 0, 0 };

			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.

			Complex c =
			{
				// idx[0] represents row
				left + (idx[0] * (right - left) / WIDTH),
				// idx[1] represents column
				top + (idx[1] * (bottom - top) / HEIGHT)
			};

			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			unsigned iterations = 0;
			while (c_abs(z) < 2.0 && iterations < max_iter)
			{
				z = c_add(c_mul(z, z), c);

				++iterations;
			}
			// set colours
			if (iterations == max_iter)
			{
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				// r, g, b values are being modified outside the lambda
				// and passed directly to the image_array_view
			}
			else
			{
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				r = iterations * iterations * r;
				g = iterations * iterations * g;
				b = iterations * iterations * b;
			}
			//unsigned int atomic_fetch_or(r << 16);
			image_array_view[idx] = (r << 16) | (g << 8) | (b);
		});
		// Implicit Synchronisation - No potential interactions amongst threads therefore none is needed
		image_array_view.synchronize(); // copy data back to CPU
	}
	catch (const Concurrency::runtime_exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
	}
	// calculate pixel image
	auto pixel_image = std::async(std::launch::async, [&]()
	{
		pixel_amp_mandelbrot_.clear();
		// generating pixel vector with mandelbrot image 
		for (int y = 0; y < HEIGHT; ++y)
		{
			for (int x = 0; x < WIDTH; ++x)
			{
				pixel_amp_mandelbrot_.push_back((image_amp_mandelbrot_[x * HEIGHT + y]) & 0xFF); // blue channel
				pixel_amp_mandelbrot_.push_back((image_amp_mandelbrot_[x * HEIGHT + y] >> 8) & 0xFF); // green channel
				pixel_amp_mandelbrot_.push_back((image_amp_mandelbrot_[x * HEIGHT + y] >> 16) & 0xFF); // red channel
			}
		}
	});
	// set calculations flag to false
	i_++;
	calculate_ = false; 
}

// No potential interactions amongst threads therefore none is needed
void Mandelbrot::amp_pixel_mandelbrot(float left, float right, float top, float bottom)
{
	/// Also observe that the parallel_for_each is not using member variables directly because that would involve
	/// marshaling the this pointer which is not allowed by one of the restrictions. 
	/// Hence the member variables are copied to local variables and then used inside the kernel.
	// extent - the extent class specifies the lenght of the data 
	// in each dimension of the array or array_view object
	// you can create an extent object and use it to create an array or array_view object
	// you can also specify	the extent using explicit parameters
	// in the array or array_view constructor
	// in this case an extent object is used to 
	// create a 2D array_view object 
	// with rows and columns defined 
	// by WIDTH and HEIGHT of the Mandelbrot set

	std::call_once(accls_pixel_mandelbrot_flag, [=]() {
		switch (current_accelerator_) {
		case NVIDIA: {
			file_amp_pixel_mandelbrot_nvidia_ << "amp_pixel_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_pixel_mandelbrot_nvidia_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case MICROSOFT_BASIC_RENDER_DRIVER: {
			file_amp_pixel_mandelbrot_msc_basic_render_driver_ << "amp_pixel_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_pixel_mandelbrot_msc_basic_render_driver_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case SOFTWARE_ADAPTER: {
			file_amp_pixel_mandelbrot_software_adapter_ << "amp_pixel_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_pixel_mandelbrot_software_adapter_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case CPU_ACCELERATOR: {
			file_amp_pixel_mandelbrot_cpu_accelerator_ << "amp_pixel_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_pixel_mandelbrot_cpu_accelerator_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		}
	});

	// accelerator to be used with parallel for each
	//accelerator_view av1 = accelerator(accelerator::default_accelerator).default_view;
	accelerator_view av = accls_[current_accelerator_].default_view;

	image_amp_pixel_mandlebrot_.empty();
	// array view - wraper for image array to calculate mandelbrot
	extent<2> image_array_view_e(WIDTH, HEIGHT);
	array_view<uint32_t, 2> image_array_view(image_array_view_e, image_amp_pixel_mandlebrot_);
	image_array_view.discard_data();

	pixel_amp_pixel_mandlebrot_.empty();
	// array view - wraper for pixel array to hold mandelbrot pixel values
	extent<1> pixel_amp_pixel_mandlebrot_e(HEIGHT * WIDTH * 3);
	array_view<int, 1> pixel_amp_pixel_mandlebrot_array_view(pixel_amp_pixel_mandlebrot_e, pixel_amp_pixel_mandlebrot_);
	pixel_amp_pixel_mandlebrot_array_view.discard_data();

	unsigned max_iter = max_iterations_;
	unsigned r = r_;
	unsigned g = g_;
	unsigned b = b_;
	// a tile - a bunch/group/block of threads (a thread block/Direct Compute - a working group/OpenCL)
	// a tile - a group of threads within the thread block
	// tiling up to 3D
	try
	{
		// kernel - code that's embeded in parallel_for_each function
		parallel_for_each(
			av,                                                      // what accelerator to use
			image_array_view.extent.tile<TILE_SIZE, TILE_SIZE>(),	 // times kernel is to tun - compute domain     
			[=]														 // pass data to computation tho' capture clause by value [=]
			(tiled_index<TILE_SIZE, TILE_SIZE> t_idx) 				 // index to access elem. of array_view
			mutable 												 // mutable allows copies to be modified, but not originals
			restrict(amp) 											 // subset of the C++ language that C++ AMP can accelerate is used
		{
			// index - represents a unique point in N-dimensional space. 
			// The index Class specifies a location in the array or array_view object 
			// (by encapsulating the offset from the origin in each dimension into one object)
			// the first parameter in the index constructor gives row number,
			// and the second parameter gives column (within row) for 2D
			index<2> idx = t_idx.global; // changes for tiled index - (latency hiding?)

			// tile_static int t[WIDTH][HEIGHT];
			// Start off z at (0, 0).
			Complex z = { 0, 0 };

			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.
			Complex c =
			{
				// idx[0] represents rows
				left + (idx[0] * (right - left) / WIDTH),
				// idx[1] represents columns
				top + (idx[1] * (bottom - top) / HEIGHT)
			};

			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			unsigned iterations = 0;
			while (c_abs(z) < 2.0 && iterations < max_iter)
			{
				z = c_add(c_mul(z, z), c);

				++iterations;
			}
			// set colours
			if (iterations == max_iter)
			{
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				r = iterations * iterations * iterations * r;
				g = iterations * iterations * iterations * g;
				b = iterations * iterations * iterations * b;
			}
			else
			{
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				r = iterations * iterations * iterations * iterations * iterations* r;
				g = iterations * iterations * iterations * iterations * iterations* g;
				b = iterations * iterations * iterations * iterations * iterations* b;
			}
			int index = (idx[0] * WIDTH + idx[1]) * 3;
			pixel_amp_pixel_mandlebrot_array_view[index] = b;
			pixel_amp_pixel_mandlebrot_array_view[index + 1] = (g << 8);
			pixel_amp_pixel_mandlebrot_array_view[index + 2] = (r << 16);

			index = (idx[0] + idx[1] * HEIGHT) * 3;
			pixel_amp_pixel_mandlebrot_array_view[index] = b;
			pixel_amp_pixel_mandlebrot_array_view[index + 1] = (g << 8);
			pixel_amp_pixel_mandlebrot_array_view[index + 2] = (r << 16);
		});
		// Implicit Synchronisation - No potential interactions amongst threads therefore none is needed
		image_array_view.synchronize(); // copy back data to CPU
	}
	catch (const Concurrency::runtime_exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
	}
	// set calculations flag to false
	i_++;
	calculate_ = false;
}

// Will not work for TILE_SIZE == 32, will work for TILE_SIZE < 32
// The smaller number of TILE_SIZE the more detailed the image is
void Mandelbrot::amp_barrier_mandelbrot(float left, float right, float top, float bottom)
{
	/// Also observe that the parallel_for_each is not using member variables directly because that would involve
	/// marshaling the this pointer which is not allowed by one of the restrictions. 
	/// Hence the member variables are copied to local variables and then used inside the kernel.
	// extent - the extent class specifies the lenght of the data 
	// in each dimension of the array or array_view object
	// you can create an extent object and use it to create an array or array_view object
	// you can also specify	the extent using explicit parameters
	// in the array or array_view constructor
	// in this case an extent object is used to 
	// create a 2D array_view object 
	// with rows and columns defined 
	// by WIDTH and HEIGHT of the Mandelbrot set

	// call once to put into file name of the function, accelerator and number of tiles
	std::call_once(accls_barrier_mandelbrot_flag, [=]() {
		switch (current_accelerator_) {
		case NVIDIA: {
				file_amp_barrier_mandelbrot_nvidia_ << "amp_barrier_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
				file_amp_barrier_mandelbrot_nvidia_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case MICROSOFT_BASIC_RENDER_DRIVER: {
				file_amp_barrier_mandelbrot_msc_basic_render_driver_ << "amp_barrier_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
				file_amp_barrier_mandelbrot_msc_basic_render_driver_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case SOFTWARE_ADAPTER: {
				file_amp_barrier_mandelbrot_software_adapter_ << "amp_barrier_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
				file_amp_barrier_mandelbrot_software_adapter_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		case CPU_ACCELERATOR: {
				file_amp_barrier_mandelbrot_cpu_accelerator_ << "amp_barrier_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
				file_amp_barrier_mandelbrot_cpu_accelerator_ << "TILE_SIZE " << TILE_SIZE << endl;
		} break;
		}
	});

	// accelerator to be used with parallel for each
	//accelerator_view av1 = accelerator(accelerator::default_accelerator).default_view;
	accelerator_view av = accls_[current_accelerator_].default_view;

	image_amp_barrier_mandelbrot_.empty();
	// array view - wraper for image array to calculate mandelbrot
	extent<2> image_array_view_e(WIDTH, HEIGHT);
	array_view<uint32_t, 2> image_array_view(image_array_view_e, image_amp_barrier_mandelbrot_);
	image_array_view.discard_data();

	// array view - wraper for pixel array to hold mandelbrot pixel values
	pixel_amp_barrier_mandelbrot_.clear();
	extent<1> pixel_amp_barrier_mandelbrot_e(HEIGHT * WIDTH * 3);
	array<int, 1> pixel_amp_barrier_mandelbrot_array(pixel_amp_barrier_mandelbrot_e, pixel_amp_barrier_mandelbrot_.begin(), pixel_amp_barrier_mandelbrot_.end());


	unsigned max_iter = max_iterations_;
	unsigned r = r_;
	unsigned g = g_;
	unsigned b = b_;
	// a tile - a bunch/group/block of threads (a thread block/Direct Compute - a working group/OpenCL)
	// a tile - a group of threads within the thread block
	// tiling up to 3D
	try
	{
		// kernel - code that's embeded in parallel_for_each function
		parallel_for_each(
			av,                                                   // what accelerator to use
			image_array_view.extent.tile<TILE_SIZE, TILE_SIZE>(), // times kernel is to tun - compute domain     
			[=, &pixel_amp_barrier_mandelbrot_array]			  // pass data to computation tho' capture clause by value [=] and pass pixel_amp_barrier_mandelbrot_array by reference [&]
			(tiled_index<TILE_SIZE, TILE_SIZE> t_idx) 			  // index to access elem. of array_view
			mutable 											  // mutable allows copies to be modified, but not originals
			restrict(amp)										  // subset of the C++ language that C++ AMP can accelerate is used
		{
			// index - represents a unique point in N-dimensional space. 
			// The index Class specifies a location in the array or array_view object 
			// (by encapsulating the offset from the origin in each dimension into one object)
			// the first parameter in the index constructor gives row number,
			// and the second parameter gives column (within row) for 2D
			index<2> idx = t_idx; // global index for image_array_view to hold row and column number

			// Start off z at (0, 0).
			Complex z = { 0, 0 };

			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.

			Complex c =
			{
				// idx[0] represents row
				left + (idx[0] * (right - left) / WIDTH),
				// idx[1] represents column
				top + (idx[1] * (bottom - top) / HEIGHT)
			};

			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			unsigned iterations = 0;
			while (c_abs(z) < 2.0 && iterations < max_iter)
			{
				z = c_add(c_mul(z, z), c);

				++iterations;
			}
			// set colours
			if (iterations == max_iter)
			{
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				// r, g, b values are being modified outside the lambda
				// and passed directly to the image_array_view
			}
			else
			{
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				r = iterations * iterations * r;
				g = iterations * iterations * g;
				b = iterations * iterations * b;
			}
			//unsigned int atomic_fetch_or(r << 16);
			image_array_view[idx] = (r << 16) | (g << 8) | (b);
			// Copy the values of the tile into a tile-sized array. 
			// create a TILE_SIZE x TILE_SIZE array to hold the values in this tile
			tile_static int tileValues[TILE_SIZE][TILE_SIZE];
			// copy the values for the tile into the TILE_SIZE x TILE_SIZE array
			tileValues[t_idx.local[1]][t_idx.local[0]] = image_array_view[t_idx];
			// when all the threads have exectuted and the TILE_SIZE x TILE_SIZE array is complete, calculate pixel array
			t_idx.barrier.wait_with_tile_static_memory_fence();

			int index = (idx[0] * HEIGHT + idx[1]) * 3;
			for (int row = 0; row < TILE_SIZE; row++) {
				for (int column = 0; column < TILE_SIZE; column++) {
					pixel_amp_barrier_mandelbrot_array[index] = tileValues[row][column];
					pixel_amp_barrier_mandelbrot_array[index + 1] = (tileValues[row][column] << 8);
					pixel_amp_barrier_mandelbrot_array[index + 2] = (tileValues[row][column] << 16);
				}
			}
		});
		try 
		{
			try 
			{
				// because conccurency::array is being used data must be explicitly copied back to the vector
				// after lambda is finished
				pixel_amp_barrier_mandelbrot_ = pixel_amp_barrier_mandelbrot_array;
			}
			catch (std::bad_alloc& x) 
			{
				cout << x.what() << endl;
			}
		}
		catch (std::bad_array_new_length& e) 
		{
			cout << e.what() << std::endl;
		}
	}
	catch (const accelerator_view_removed & ex)
	{
		cout << ex.what() << endl;
		cout << ex.get_view_removed_reason() << endl;
	}
	catch (const Concurrency::runtime_exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_ICONERROR);
	}
	// set calculations flag to false
	i_++;
	calculate_ = false;
}

void Mandelbrot::update(float dt)
{
	//std::ofstream outputFile("mandelbrot_timing__.csv");
	// list all accelerators only once when the programm starts
	std::call_once(flag_, [=]()
	{
		//get all accelerators available to us and store in a vector so we can extract details
		std::vector<accelerator> accls = accelerator::get_all();
		if (accls.empty())
		{
			cout << "No accelerators found that are compatible with C++ AMP" << std::endl;
		}
		else
		{
			cout << "Accelerators found that are compatible with C++ AMP" << std::endl;
			// iterates over all accelerators and print characteristics
			for (unsigned i = 0; i<accls.size(); i++)
			{
				std::wcout << " acc " << i + 1 << " = " << accls[i].description << endl;
				if (accls[i] == accelerator(accelerator::direct3d_ref))
					std::cout << " WARNING!! Running on very slow emulator! Only use this accelerator for debugging." << std::endl;

				const std::wstring bs[2] = { L"false", L"true" };
				std::wcout << ": " << accls[i].description << " "
					<< endl << "       device_path                       = " << accls[i].device_path
					<< endl << "       dedicated_memory                  = " << std::setprecision(4) << float(accls[i].dedicated_memory) / (1024.0f * 1024.0f) << " Mb"
					<< endl << "       has_display                       = " << bs[accls[i].has_display]
					<< endl << "       is_debug                          = " << bs[accls[i].is_debug]
					<< endl << "       is_emulated                       = " << bs[accls[i].is_emulated]
					<< endl << "       supports_double_precision         = " << bs[accls[i].supports_double_precision]
					<< endl << "       supports_limited_double_precision = " << bs[accls[i].supports_limited_double_precision]
					<< endl << endl;
			}
		}
	});

	const float scale_per = 0.1f * zoom_scale_;
	// mouse wheel up
	if (input->isScrollDownMouseWheel())
	{
		scale_.subtract(zoom_, zoom_scale_);
		input->setScrollDownMouseWheel(false);
	}
	// mouse wheel down
	if (input->isScrollUpMouseWheel())
	{
		scale_.add(zoom_, zoom_scale_);
		input->setScrollUpMouseWheel(false);
	}
	// increase: number of maximum iterations; red, green, blue colour values; and recalculate Mandelbrot
	if (input->isKeyDown('u') ||
		input->isKeyDown('U'))
	{
		++max_iterations_;
		if (r_ < 255) { ++r_; }
		if (g_ < 255) { ++g_; }
		if (b_ < 255) { ++b_; }
		if (r_ == 255) { r_ = 0; }
		if (g_ == 255) { g_ = 0; }
		if (b_ == 255) { b_ = 0; }
		calculate_ = true;
	}
	// decrease: number of maximum iterations; red, green, blue colour values; and recalculate Mandelbrot
	if (input->isKeyDown('i') ||
		input->isKeyDown('I'))
	{
		if (max_iterations_ > 0) { --max_iterations_; }
		if (r_ > 0) { --r_; }
		if (g_ > 0) { --g_; }
		if (b_ > 0) { --b_; }
		if (r_ == 0) { r_ = 255; }
		if (g_ == 0) { g_ = 255; }
		if (b_ == 0) { b_ = 255; }
		calculate_ = true;
	}
	// increase: number of maximum iterations; red, green, blue colour values; and recalculate Mandelbrot
	if (input->isKeyDown('o') ||
		input->isKeyDown('O'))
	{
		if (r_ < 255) { ++r_; }
		if (g_ < 255) { ++g_; }
		if (b_ < 255) { ++b_; }
		if (r_ == 255) { r_ = 0; }
		if (g_ == 255) { g_ = 0; }
		if (b_ == 255) { b_ = 0; }
		calculate_ = true;
	}
	// decrease: number of maximum iterations; red, green, blue colour values; and recalculate Mandelbrot
	if (input->isKeyDown('p') ||
		input->isKeyDown('P'))
	{
		if (r_ > 0) { --r_; }
		if (g_ > 0) { --g_; }
		if (b_ > 0) { --b_; }
		if (r_ == 0) { r_ = 255; }
		if (g_ == 0) { g_ = 255; }
		if (b_ == 0) { b_ = 255; }
		calculate_ = true;
	}
	// increase number of maximum iterations
	if (input->isKeyDown('z') ||
		input->isKeyDown('Z'))
	{
		max_iterations_ += 1;
		cout << max_iterations_ << endl;
	}
	// increase number of maximum iterations (can't go lower than 0)
	if (input->isKeyDown('x') ||
		input->isKeyDown('X'))
	{
		if (max_iterations_ > 0) { max_iterations_ -= 1; }
		cout << max_iterations_ << endl;
	}
	// left arrow increase red colour value
	if (input->isSpecialKeyDown(GLUT_KEY_LEFT))
	{
		if (r_ < 255) { ++r_; }
		cout << "red: " << r_ << endl;
		calculate_ = true;
	}
	// right arrow increase green colour value
	if (input->isSpecialKeyDown(GLUT_KEY_RIGHT))
	{
		if (g_ < 255) { ++g_; }
		cout << "green: " << g_ << endl;
		calculate_ = true;
	}
	// up arrow increase blue colour value
	if (input->isSpecialKeyDown(GLUT_KEY_UP))
	{
		if (b_ < 255) { ++b_; }
		cout << "blue: " << b_ << endl;
		calculate_ = true;
	}
	// down arrow and 'r' key to decrease red colour value
	if ((input->isKeyDown('r') && input->isSpecialKeyDown(GLUT_KEY_DOWN)) ||
		(input->isKeyDown('R') && input->isSpecialKeyDown(GLUT_KEY_DOWN)))
	{
		if (r_ > 0) { --r_; }
		cout << "red: " << r_ << endl;
		calculate_ = true;
	}
	// down arrow and 'g' key to decrease green colour value
	if ((input->isKeyDown('g') && input->isSpecialKeyDown(GLUT_KEY_DOWN)) ||
		(input->isKeyDown('G') && input->isSpecialKeyDown(GLUT_KEY_DOWN)))
	{
		if (g_ > 0) { --g_; }
		cout << "green: " << g_ << endl;
		calculate_ = true;
	}
	// down arrow and 'b' key to decrease blue colours value
	if ((input->isKeyDown('b') && input->isSpecialKeyDown(GLUT_KEY_DOWN)) ||
		(input->isKeyDown('B') && input->isSpecialKeyDown(GLUT_KEY_DOWN)))
	{
		if (b_ > 0) { --b_; }
		cout << "blue: " << b_ << endl;
		calculate_ = true;
	}
	// display current values of red, green, blue colours and maximum iterations
	if (input->isKeyDown('l') ||
		input->isKeyDown('L'))
	{
		cout
		<< endl << "red: " << r_ 
		<< endl << "green: " << g_ 
		<< endl << "blue: " << b_ 
		<< endl << "iterations: " << max_iterations_ 
		<< endl << endl;
		input->SetKeyUp('l'); 
		input->SetKeyUp('L');
	}
	// calculate the Mandelbrot set multiple times (equal to max_timings_ value)
	if (input->isKeyDown('c') ||
		input->isKeyDown('C'))
	{
		cout << "Calculating...\n";
		timing_ = true;
		input->SetKeyUp('c');
		input->SetKeyUp('C');
	}
	// calculate the Mandelbrot once
	if (input->isKeyDown('v') ||
		input->isKeyDown('V'))
	{
		cout << "Calculating once...\n";
		calculate_ = true;
		input->SetKeyUp('v');
		input->SetKeyUp('V');
	}
	// use NVIDIA accelerator with current Mandelbrot
	if (input->isKeyDown('1'))
	{
		// set current accelerator to NVIDIA
		current_accelerator_ = 0;
		// depending on the current Mandelbrot method being used put timing results into separate files
		switch (calc_mandelbrot_)
		{
		case AMP_MANDELBROT :
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_mandelbrot_nvidia_ << "amp_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_mandelbrot_nvidia_ << "TILE_SIZE " << TILE_SIZE << endl;

			break;
		case AMP_PIXEL_MANDELBROT :
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_pixel_mandelbrot_nvidia_ << "amp_pixel_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_pixel_mandelbrot_nvidia_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		case AMP_BARRIER_MANDELBROT :
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_barrier_mandelbrot_nvidia_ << "amp_barrier_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_barrier_mandelbrot_nvidia_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		}
		input->SetKeyUp('1');
	}
	// use Microsoft basic render driver accelerator with current Mandelbrot
	if (input->isKeyDown('2'))
	{
		// set current accelerator to Microsoft basic render driver
		current_accelerator_ = 1;
		// depending on the current Mandelbrot method being used put timing results into separate files
		switch (calc_mandelbrot_)
		{
		case AMP_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_mandelbrot_msc_basic_render_driver_ << "amp_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_mandelbrot_msc_basic_render_driver_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		case AMP_PIXEL_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_pixel_mandelbrot_msc_basic_render_driver_ << "amp_pixel_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_pixel_mandelbrot_msc_basic_render_driver_ << "TILE_SIZE " << TILE_SIZE << endl;

			break;
		case AMP_BARRIER_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_barrier_mandelbrot_msc_basic_render_driver_ << "amp_barrier_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_barrier_mandelbrot_msc_basic_render_driver_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		}
		input->SetKeyUp('2');
	}
	// use software adapter accelerator with current Mandelbrot
	if (input->isKeyDown('3'))
	{
		// set current accelerator to software adapter
		current_accelerator_ = 2;
		// depending on the current Mandelbrot method being used put timing results into separate files
		switch (calc_mandelbrot_)
		{
		case AMP_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			if (accls_[current_accelerator_] == accelerator(accelerator::direct3d_ref))
				std::cout << " WARNING!! Running on very slow emulator! Only use this accelerator for debugging." << std::endl;
			file_amp_mandelbrot_software_adapter_ << "amp_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_mandelbrot_software_adapter_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		case AMP_PIXEL_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			if (accls_[current_accelerator_] == accelerator(accelerator::direct3d_ref))
				std::cout << " WARNING!! Running on very slow emulator! Only use this accelerator for debugging." << std::endl;
			file_amp_pixel_mandelbrot_software_adapter_ << "amp_pixel_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_pixel_mandelbrot_software_adapter_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		case AMP_BARRIER_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			if (accls_[current_accelerator_] == accelerator(accelerator::direct3d_ref))
				std::cout << " WARNING!! Running on very slow emulator! Only use this accelerator for debugging." << std::endl;
			file_amp_barrier_mandelbrot_software_adapter_ << "amp_barrier_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_barrier_mandelbrot_software_adapter_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		}
		input->SetKeyUp('3');
	}
	// use cpu accelerator with current Mandelbrot
	if (input->isKeyDown('4'))
	{
		// set current accelerator to cpu accelerator
		current_accelerator_ = 3;
		// depending on the current Mandelbrot method being used put timing results into separate files
		switch (calc_mandelbrot_)
		{
		case AMP_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_mandelbrot_cpu_accelerator_ << "amp_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_mandelbrot_cpu_accelerator_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		case AMP_PIXEL_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_pixel_mandelbrot_cpu_accelerator_ << "amp_pixel_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_pixel_mandelbrot_cpu_accelerator_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		case AMP_BARRIER_MANDELBROT:
			std::wcout << "Using acc " << current_accelerator_ + 1 << " = " << accls_[current_accelerator_].description << endl;
			file_amp_barrier_mandelbrot_cpu_accelerator_ << "amp_barrier_mandelbrot using " << ws2s(accls_[current_accelerator_].description) << endl;
			file_amp_barrier_mandelbrot_cpu_accelerator_ << "TILE_SIZE " << TILE_SIZE << endl;
			break;
		}
		input->SetKeyUp('4');
	}
	// switch to amp_mandelbrot Mandelbrot calculation method
	if (input->isKeyDown('5'))
	{
		calc_mandelbrot_ = AMP_MANDELBROT;
		cout << "\nDisplaying amp_madelbrot set\n" << endl;
		input->SetKeyUp('5');
	}
	// switch to amp_pixel_mandelbrot Mandelbrot calculation method
	if (input->isKeyDown('6'))
	{
		calc_mandelbrot_ = AMP_PIXEL_MANDELBROT;
		cout << "\nDisplaying amp_pixel_madelbrot set\n" << endl;
		input->SetKeyUp('6');
	}
	// switch to amp_barrier_mandelbrot Mandelbrot calculation method
	if (input->isKeyDown('7'))
	{
		calc_mandelbrot_ = AMP_BARRIER_MANDELBROT;
		cout << "\nDisplaying amp_barrier_madelbrot set\n" << endl;
		input->SetKeyUp('7');
	}
	// after 'c' was pressed keep calculating the Mandelbrot set until i_ reaches the maximum amount of timigns (max_timings_)
	if (i_ == max_timings_)
	{
		timing_ = false;
		i_ = 0;
		//exit(0);
	}
	// calculate the Mandelbrot set only when the function was called
	if (calculate_ || timing_)
	{
		// <------------------------------------------------------------- start timing
		auto start_timing = std::async(std::launch::async, [=]()
		{
			if (accls_[current_accelerator_] == accelerator(accelerator::direct3d_ref))
				cout << "Calculating Mandelbrot..." << endl; 
			// Start timing
			the_clock::time_point start = the_clock::now();
			return start;
		});

		switch (calc_mandelbrot_)
		{
		case AMP_MANDELBROT :
		{
		// This shows the whole set.
			//cpu_mandelbrot(-2.0, 1.0, 1.125, -1.125); // 31630, 27280, 27083 [ms]
			amp_mandelbrot(-2.0, 1.0, 1.125, -1.125); // 59, 112, 110, 64 [ms]
		} break;
		case AMP_PIXEL_MANDELBROT:
		{
			// This shows the whole set.
			amp_pixel_mandelbrot(-2.0, 1.0, 1.125, -1.125); // 59, 112, 110, 64 [ms]
		} break;
		case AMP_BARRIER_MANDELBROT :
		{
		// This shows the whole set.
			amp_barrier_mandelbrot(-2.0, 1.0, 1.125, -1.125); // 59, 112, 110, 64 [ms]
		} break;
		}
		// <---------------------------------------------------------end timing
		auto end_timing = std::async(std::launch::async, [=, &start_timing]()
		{
			the_clock::time_point start = start_timing.get();
			the_clock::time_point end = the_clock::now();
			// Compute the difference between the two times in milliseconds
			auto time_taken = duration_cast<milliseconds>(end - start).count();
			
			// put timings into files depending on the current Mandelbot set calculation mathod and accelerator
			if (timing_) 
			{
				switch (calc_mandelbrot_)
				{
				case AMP_MANDELBROT:
				{
					switch (current_accelerator_) {
					case NVIDIA: {
						file_amp_mandelbrot_nvidia_ << time_taken << endl;
					} break;
					case MICROSOFT_BASIC_RENDER_DRIVER: {
						file_amp_mandelbrot_msc_basic_render_driver_ << time_taken << endl;
					} break;
					case SOFTWARE_ADAPTER: {
						file_amp_mandelbrot_software_adapter_ << time_taken << endl;
					} break;
					case CPU_ACCELERATOR: {
						file_amp_mandelbrot_cpu_accelerator_ << time_taken << endl;
					} break;
					}
					std::cout << i_ << "\n";
				} break;
				case AMP_PIXEL_MANDELBROT:
				{
					switch (current_accelerator_) {
					case NVIDIA: {
						file_amp_pixel_mandelbrot_nvidia_ << time_taken << endl;
					} break;
					case MICROSOFT_BASIC_RENDER_DRIVER: {
						file_amp_pixel_mandelbrot_msc_basic_render_driver_ << time_taken << endl;
					} break;
					case SOFTWARE_ADAPTER: {
						file_amp_pixel_mandelbrot_software_adapter_ << time_taken << endl;
					} break;
					case CPU_ACCELERATOR: {
						file_amp_pixel_mandelbrot_cpu_accelerator_ << time_taken << endl;
					} break;
					}
					std::cout << i_ << "\n";
				} break;
				case AMP_BARRIER_MANDELBROT:
				{
					switch (current_accelerator_) {
					case NVIDIA: {
						file_amp_barrier_mandelbrot_nvidia_ << time_taken << endl;
					} break;
					case MICROSOFT_BASIC_RENDER_DRIVER: {
						file_amp_barrier_mandelbrot_msc_basic_render_driver_ << time_taken << endl;
					} break;
					case SOFTWARE_ADAPTER: {
						file_amp_barrier_mandelbrot_software_adapter_ << time_taken << endl;
					} break;
					case CPU_ACCELERATOR: {
						file_amp_barrier_mandelbrot_cpu_accelerator_ << time_taken << endl;
					} break;
					}
					std::cout << i_ << "\n";
				} break;
				}
			} // display single timings
			else 
			{
				std::wcout << "Computing Mandelbrot using " << accls_[current_accelerator_].description
						   << " took " << time_taken << " ms." << endl;
			}
		});
	}
	// update the camera
	camera->cameraControll(dt, WIDTH, HEIGHT, input);
	camera->update();
}

void Mandelbrot::render()
{
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();
	// Set the camera
	//gluLookAt(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	gluLookAt(camera->getPositionX(), camera->getPositionY(), camera->getPositionZ(),
		camera->getLookAtX(), camera->getLookAtY(), camera->getLookAtZ(),
		camera->getUpX(), camera->getUpY(), camera->getUpZ());
	
	switch (calc_mandelbrot_)
	{
	case AMP_MANDELBROT :
	{
		glPushMatrix(); {
			// Scale
			glScalef(scale_.x, scale_.y, scale_.z);
			// Translate
			glTranslatef(translate_.x, translate_.y, translate_.z);
			// render Mandelbrot
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			glVertexPointer(3, GL_FLOAT, 0, quad_t_verts.data());
			glTexCoordPointer(2, GL_FLOAT, 0, quad_t_texcoords.data());

			glGenTextures(1, &amp_mandelbrot_texture_);
			glBindTexture(GL_TEXTURE_2D, amp_mandelbrot_texture_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, WIDTH, HEIGHT,
				0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_amp_mandelbrot_.data()); // <----- had to use GL_BGR_EXT

			//glColor4f(_rgba.getR(), _rgba.getG(), _rgba.getB(), _rgba.getA());
			glDrawArrays(GL_TRIANGLES, 0, quad_t_verts.size() / 3);
			//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glBindTexture(GL_TEXTURE_2D, NULL);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		} glPopMatrix();
	} break;

	case AMP_PIXEL_MANDELBROT:
	{
		glPushMatrix(); {
			// Scale
			glScalef(scale_.x, scale_.y, scale_.z);
			// Translate
			glTranslatef(translate_.x, translate_.y, translate_.z);
			// render Mandelbrot
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			glVertexPointer(3, GL_FLOAT, 0, quad_t_verts.data());
			glTexCoordPointer(2, GL_FLOAT, 0, quad_t_texcoords.data());

			glGenTextures(1, &amp_barrier_mandelbrot_texture_);
			glBindTexture(GL_TEXTURE_2D, amp_barrier_mandelbrot_texture_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, WIDTH, HEIGHT,
				0, GL_BGR_EXT, GL_INT, pixel_amp_pixel_mandlebrot_.data()); // <----- had to use GL_INT

			glDrawArrays(GL_TRIANGLES, 0, quad_t_verts.size() / 3);
			glBindTexture(GL_TEXTURE_2D, NULL);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		} glPopMatrix();
	} break;

	case AMP_BARRIER_MANDELBROT :
	{
		glPushMatrix(); {
			// Scale
			glScalef(scale_.x, scale_.y, scale_.z);
			// Translate
			glTranslatef(translate_.x, translate_.y, translate_.z);
			// render Mandelbrot
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			glVertexPointer(3, GL_FLOAT, 0, quad_t_verts.data());
			glTexCoordPointer(2, GL_FLOAT, 0, quad_t_texcoords.data());

			glGenTextures(1, &amp_pixel_mandelbrot_texture_);
			glBindTexture(GL_TEXTURE_2D, amp_pixel_mandelbrot_texture_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, WIDTH, HEIGHT,
				0, GL_BGR_EXT, GL_INT, pixel_amp_barrier_mandelbrot_.data()); // <----- had to use GL_INT

			glDrawArrays(GL_TRIANGLES, 0, quad_t_verts.size() / 3);
			glBindTexture(GL_TEXTURE_2D, NULL);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		} glPopMatrix();
	} break;
	}
}