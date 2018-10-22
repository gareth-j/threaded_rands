#ifndef THREADEDRANDS_HPP
#define	THREADEDRANDS_HPP

#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <string>
#include <memory>
#include <thread>
#include <cstdlib>
#include <omp.h>

#include "generators.hpp"

enum class generator_type{xoro128, pcg64, jsf64};

class Threaded_rands
{	
public:
	Threaded_rands(const unsigned int _n_threads = 1)
	{
		n_threads = get_thread_info(_n_threads);

		for(int thread_id = 0; thread_id < n_threads; thread_id++)
        	gen_vec.push_back(std::make_unique<pcg64_wrap>(thread_id));
	}

	Threaded_rands(const generator_type sel, const unsigned int _n_threads)
	{			
		// Check the number of threads passed isn't greater than the number available in hardware
		// If passed is > hardware threads, uses max available in hardware
		// TODO - Implement override for this
		n_threads = get_thread_info(_n_threads);

		for(int thread_id = 0; thread_id < n_threads; thread_id++)
        {
	        if(sel == generator_type::pcg64)
	        	gen_vec.push_back(std::make_unique<pcg64_wrap>(thread_id));
			if(sel == generator_type::xoro128)
	        	gen_vec.push_back(std::make_unique<xoroshiro128>(thread_id));
	        if(sel == generator_type::jsf64)
	        	gen_vec.push_back(std::make_unique<jsf64_wrap>(thread_id));
	    }
    }

    // Get 64-bit rand from generator
    uint64_t get_rand(const unsigned int thread_id);

    uint64_t operator()() { return get_rand(0); }

	// Uses Lemire's method to quickly get a rand within a range
	uint64_t get_bounded_rand(const uint64_t upper, const unsigned int thread_id);
	

	// TODO - 32-bit generation of ranged values
	// 64-bit numbers for small ranges is overkill

	// Fills a vector with 64-bit ints [lower:upper)
	void generate_range(std::vector<uint64_t>& vec, const unsigned int lower, const unsigned int upper, const unsigned int thread_id = 0);
	// Fills a 2D vector with 64-bit ints [lower:upper)
	void generate_range(std::vector<std::vector<uint64_t>>& vec, const unsigned int lower, const unsigned int upper);

	// Note - the method below result in a difference of ~ 1e-8 from dividing by UINT64_MAX	
	// Check if this introduces a bias of some kind in the generated numbers.

	// Creates a double in the range [0:1) from a 64-bit unsigned int
	inline double get_double(const uint64_t v) {return ((uint64_t)(v >> 11)) / (double)(1L << 53);}

	// Alternative - but slower in my measurements - versions that may be more precise / not have rounding errors ?
	// inline double get_double64(const uint64_t v) {return v/double(UINT64_MAX);}

	// TODO - implement templated functions for generation of both 64-bit and 32-bit PRNGs

	// Take an array or vector and fill with 64-bit rands
	// Generate 64-bit
	void generate(std::vector<uint64_t>& vec, const unsigned int thread_id = 0)
	{
		for(auto& rand : vec)
			rand = get_rand(thread_id);
	}

	// For 2D vectors
	// Although any type can be passed here there will be an implicit conversion from uint64_t to T
	template<typename T>
	void generate_2D(std::vector<T>& vec)
	{
		// Disable dynamic teams
		omp_set_dynamic(0);
		#pragma omp parallel for num_threads(n_threads)
		for(unsigned int i = 0; i < vec.size(); i++)
		{
			generate(vec[i], i);
		}
	}

	// For 2D arrays - shouldn't need higher dimensions than 2D - this is just for multiple threads
	template<typename T, const std::size_t N>
	void generate_2D(std::array<T, N>& arr)
	{
		// Disable dynamic teams
		omp_set_dynamic(0);

		#pragma omp parallel for num_threads(n_threads)
		for(unsigned int i = 0; i < arr.size(); i++)
		{
			generate(arr[i], i);
		}
	}

	// Fills a one-dimensional vector with rands in the range [0, 1)
	void generate_doubles(std::vector<double>& vec, const unsigned int thread_id = 0)
	{
		for(auto& rand : vec)
			rand = get_double(get_rand(thread_id));			
	}

	// Fills the vector with values [0:1)
	template <typename T>
	void generate_doubles_2D(std::vector<T>& vec)
	{
		// Disable dynamic teams
		omp_set_dynamic(0);

		#pragma omp parallel for num_threads(n_threads)
		for(unsigned int i = 0; i < vec.size(); i++)
		{
			generate_doubles(vec[i], i);
		}
	}

	// Fills the vector with values [0:1)
	template <typename T, const std::size_t N>
	void generate_doubles_2D(std::array<T, N>& vec)
	{
		// Disable dynamic teams
		omp_set_dynamic(0);

		#pragma omp parallel for num_threads(n_threads)
		for(unsigned int i = 0; i < vec.size(); i++)
		{
			generate_doubles(vec[i], i);
		}
	}

	~Threaded_rands() {}	

private:
	unsigned int get_thread_info(const int nt);
	unsigned int backup_thread_count();

	unsigned int n_threads = 1;		

	std::vector<std::unique_ptr<base_class>> gen_vec;

}; // End class

// ======================================
// Threaded_rands member functions
// ======================================

uint64_t Threaded_rands::get_rand(const unsigned int thread_id)
{
	return gen_vec[thread_id]->get_rand();
}

// Lemire's method with modification by Prof. M. O'Neil
// http://www.pcg-random.org/posts/bounded-rands.html
// Can use this for upper and lower bounds as in generate_range
uint64_t Threaded_rands::get_bounded_rand(const uint64_t upper, const unsigned int thread_id)
{
	int range = upper;

	uint64_t x = get_rand(thread_id);
    
    __uint128_t m = static_cast<__uint128_t>(x) * static_cast<__uint128_t>(range);
    
    uint64_t l = static_cast<uint64_t>(m);

    if (l < range) 
    {
        uint64_t t = -range;
        if (t >= range) 
        {
            t -= range;
            if (t >= range) 
                t %= range;
        }
        while (l < t) 
        {
            x = get_rand(thread_id);
            m = __uint128_t(x) * __uint128_t(range);
            l = static_cast<uint64_t>(m);
        }
    }

    return m >> 64;
}

// This is currently limited to positive 64-bit ints - should it be the full range of the generator and negatives?
void Threaded_rands::generate_range(std::vector<uint64_t>& vec, 
									const unsigned int lower, const unsigned int upper, const unsigned int thread_id)
{
	// Use this for the generator and then modify what we get to get the correct range	
	int our_upper = upper - lower;
	// Add this value to the values returned by the generator to scale the range
	uint64_t scale_value = lower;

	for(auto& i : vec)
	{
		uint64_t a_rand = get_bounded_rand(our_upper, thread_id);
		i = a_rand + scale_value;
	}
}

// This is currently limited to positive 64-bit ints - should it be the full range of the generator and negatives?
void Threaded_rands::generate_range(std::vector<std::vector<uint64_t>>& vec, 
									const unsigned int lower, const unsigned int upper)
{
	// Use this for the generator and then modify what we get to get the correct range	
	int our_upper = upper - lower;
	// Add this value to the values returned by the generator to scale the range
	uint64_t scale_value = lower;

	// Disable dynamic teams
	omp_set_dynamic(0);

	#pragma omp parallel for num_threads(n_threads)
	for(unsigned int i = 0; i < vec.size(); i++)
	{
		generate_range(vec[i], lower, upper, i);
	}


}

// Detect the number of threads available on the machine using (if needed) multiple methods 
unsigned int Threaded_rands::get_thread_info(const int n_selected)
{

	// How many to use
	unsigned int n_utilise = 1;
	// How many we have available
	unsigned int n_hardware = std::thread::hardware_concurrency();

	// If the above doesn't work (it will return 0 on error) use a different method
	if(n_hardware == 0)
		n_hardware = backup_thread_count();
	
	if(n_selected <= 0)
	{
		std::cerr << "The number of selected threads is too low. The maximum available in hardware will be used.\n";
		n_utilise = n_hardware;
	}
	else if(n_selected > n_hardware)
	{
		std::cerr << "Number of requested threads exceeds those available in hardware, using max available.\n";
		n_utilise = n_hardware;
	}
	else
		n_utilise = n_selected;

	return n_utilise;
}

// // If hardware_concurrency method fails fall back to this and detect the type of OS
// // being used for a more portable (?) solution
unsigned int Threaded_rands::backup_thread_count()
{
	unsigned int n_threads = 1;

// For 64-bit Windows
#if defined(_WIN64)
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	n_threads = sysinfo.dwNumberOfProcessors;
#elif defined(__linux__)
	n_threads = sysconf(_SC_NPROCESSORS_ONLN);
#endif

	return n_threads;
}

#endif