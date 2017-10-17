#pragma once
#include <iostream>
#include <chrono>
#include <complex>
#include <time.h>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <array>

// Import things we need from the standard library
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::complex;
using std::cout;
using std::endl;
using std::ofstream;
// Define the alias "the_clock" for the clock type we're going to use.
typedef std::chrono::steady_clock the_clock;
