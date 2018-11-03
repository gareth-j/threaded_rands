#ifndef THREADEDRANDS_HPP
#define	THREADEDRANDS_HPP

#include <iostream>
#include <vector>
#include <random>
#include <memory>
#include <type_traits>
#include <cstdlib>
#include <omp.h>

#include "generators.hpp"

enum class generator_type{xoro128, pcg64, jsf64};

template<typename result_type, typename state_type>
class Threaded_rands
{
protected:
	// Gets hardware thread information - can be used to limit
	// the number of threads requested to the number available in hardware
	unsigned int get_thread_info(const int nt);
	// In case C++11 hardware detection methods fail
	unsigned int backup_thread_count();
	// Number of threads to be used
	unsigned int n_threads = 1;	

	// Store the PRNG object created for each thread
	std::vector<std::unique_ptr<base_class>> gen_vec;

	// Number of bits in each type
    const unsigned int RTYPE_BITS = 8*sizeof(result_type);
    const unsigned int STYPE_BITS = 8*sizeof(state_type);

	// This is used in the conversion of int types
	const unsigned int bit_shift = STYPE_BITS - RTYPE_BITS;

	// For int to double conversion using the quick method
	const unsigned int right_shift = ((RTYPE_BITS == 64) ? 11 : 9);
	const unsigned int left_shift = ((RTYPE_BITS == 64) ? 53 : 23);

	// Check to see if 16-bit types are requested
	static_assert(sizeof(result_type) < 32, "16-bit types not currently supported.");

	// Pick the type of int we want to use for getting bounded rands
	// This needs a better name
	using int_type = typename std::conditional<sizeof(state_type) == 64, __uint128_t, uint64_t>::type;


public:
	// Min max values that can be output by the object	
    static constexpr result_type min() { return 0; }
    // Where ~ performs a bitwise NOT on zero to get the max of that type
    static constexpr result_type max() { return ~ result_type(0); }

    // Default ctor, using the PCG64 PRNG and a single thread
    Threaded_rands(const int n = 1)
    {  	// n_threads = get_thread_info(n);
		// set_bit_shifts();
		// if(RTYPE_BITS == 64)
		// {
			for(int thread_id = 0; thread_id < n; thread_id++)
	        	gen_vec.push_back(std::make_unique<pcg64_wrap>(thread_id));     		
		// }
		// else if(RTYPE_BITS == 32)
		// 	for(int thread_id = 0; thread_id < n; thread_id++)
	 //        	gen_vec.push_back(std::make_unique<pcg64_wrap>(thread_id));
	}

	// Threaded_rands(const generator_type sel, const unsigned int _n_threads)
	// {			
	// 	// Check the number of threads passed isn't greater than the number available in hardware
	// 	// If passed is > hardware threads, uses max available in hardware
	// 	// TODO - Implement override for this
	// 	// n_threads = get_thread_info(_n_threads);
	    
	//  //    set_bit_shifts();

	// 	// Get working with pcg32 and pcg64 initially

	// 	for(int thread_id = 0; thread_id < n_threads; thread_id++)
 //        {
	//         if(sel == generator_type::pcg64)
	//         	gen_vec.push_back(std::make_unique<pcg64_wrap>(thread_id));
	// 		if(sel == generator_type::xoro128)
	//         	gen_vec.push_back(std::make_unique<xoroshiro128>(thread_id));
	//         if(sel == generator_type::jsf64)
	//         	gen_vec.push_back(std::make_unique<jsf64_wrap>(thread_id));
	//     }

 //    }


    result_type get_rand(const unsigned int thread_id)
    {    	
    	state_type rand = gen_vec[thread_id]->get_rand();
   		
   		// If we're using a 64-bit generator for 32-bit ints
    	return rand >> bit_shift;
    }
    // Just use the first thread if operator() is called
    result_type operator()(){return get_rand(0);}
	
    // Uses Lemire's method to quickly get a rand within a range
    // For [0:upper)
	result_type get_bounded_rand(const result_type upper, const unsigned int thread_id);	
	// For [lower:upper)
	result_type get_bounded_rand(const result_type lower, const result_type upper, const unsigned int thread_id);

	// Fills a vector with 64-bit ints [lower:upper)
	void generate_range(std::vector<result_type>& vec, const unsigned int upper, const unsigned int thread_id);
	void generate_range(std::vector<result_type>& vec, const unsigned int lower, const unsigned int upper, const unsigned int thread_id = 0);
	
	// Fills a 2D vector with 64-bit ints [lower:upper)
	void generate_range(std::vector<std::vector<result_type>>& vec, const unsigned int upper);
	void generate_range(std::vector<std::vector<result_type>>& vec, const unsigned int lower, const unsigned int upper);

	// This function currently just returns a double in the range [0:1)
	// This method below result in a difference of ~ 1e-8 from dividing by UINT64_MAX	
	// TODO - Check if this introduces a bias in the generated numbers.
	inline double get_double(const result_type v) {return ((uint64_t)(v >> right_shift)) / (double)(1L << left_shift);}
	
	// Alternative - but slower in my measurements - versions that may be more precise / not have rounding errors ?
	// inline double get_double(const result_type v) { return v/double(max());}

	// TODO - add in functions for a better range of doubles  

	// Take an array or vector and fill with 64-bit rands
	// Generate 64-bit

	// PROBLEMO
	// What if the state type and result type are different, then this doesn't work. 
	
	void generate(std::vector<result_type>& vec, const unsigned int thread_id = 0)
	{
		for(auto& rand : vec)
			rand = get_rand(thread_id);
	}

	// For 2D vectors
	// Although any type can be passed here there will be an implicit conversion from uint64_t to T
	template<typename T>
	void generate_2D(std::vector<T>& vec)
	{
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
		#pragma omp parallel for num_threads(n_threads)
		for(unsigned int i = 0; i < vec.size(); i++)
		{
			generate_doubles(vec[i], i);
		}
	}

}; // End class

// ======================================
// Threaded_rands member functions
// ======================================

// Lemire's method with modification by Prof. M. O'Neil
// http://www.pcg-random.org/posts/bounded-rands.html
// Can use this for upper and lower bounds as in generate_range

// For random numbers in a range [0:upper)
template<typename result_type, typename state_type>
result_type Threaded_rands<result_type, state_type>::get_bounded_rand(const result_type upper, const unsigned int thread_id)
{
	// State type is the type returned from the generator
	state_type x = get_rand(thread_id);
    
    int_type m = static_cast<int_type>(x) * static_cast<int_type>(upper);
    
    state_type l = static_cast<state_type>(m);

    if (l < upper) 
    {
        state_type t = -upper;
        if (t >= upper) 
        {
            t -= upper;
            if (t >= upper) 
                t %= upper;
        }
        while (l < t) 
        {
            x = get_rand(thread_id);
            m = int_type(x) * int_type(upper);
            l = static_cast<state_type>(m);
        }
    }

    return m >> bit_shift;
}

// For random numbers in a range [lower:upper)
template<typename result_type, typename state_type>
result_type Threaded_rands<result_type, state_type>::get_bounded_rand(const result_type lower, const result_type upper, const unsigned int thread_id)
{
	// If we're not doing [0-n)
	result_type our_upper = upper - lower;

	// State type is the type returned from the generator
	state_type x = get_rand(thread_id);
    
    int_type m = static_cast<int_type>(x) * static_cast<int_type>(our_upper);
    
    state_type l = static_cast<state_type>(m);

    if (l < our_upper) 
    {
        state_type t = -our_upper;
        if (t >= our_upper) 
        {
            t -= our_upper;
            if (t >= our_upper) 
                t %= our_upper;
        }
        while (l < t) 
        {
            x = get_rand(thread_id);
            m = int_type(x) * int_type(our_upper);
            l = static_cast<state_type>(m);
        }
    }

    // Add lower to get within the correct range
    m += lower;

    // Shift to get the type requested
    return m >> bit_shift;
}

// This is currently limited to positive 64-bit ints - should it be the full range of the generator and negatives?
template<typename result_type, typename state_type>
void Threaded_rands<result_type, state_type>::generate_range(std::vector<result_type>& vec, const unsigned int upper, const unsigned int thread_id)
{
	for(auto& i : vec)
		i = get_bounded_rand(upper, thread_id);
}

template<typename result_type, typename state_type>
void Threaded_rands<result_type, state_type>::generate_range(std::vector<result_type>& vec, const unsigned int lower, const unsigned int upper, const unsigned int thread_id)
{
	for(auto& i : vec)
		i = get_bounded_rand(lower, upper, thread_id);
}

template<typename result_type, typename state_type>
void Threaded_rands<result_type, state_type>::generate_range(std::vector<std::vector<result_type>>& vec, const unsigned int upper)
{
	#pragma omp parallel for num_threads(n_threads)
	for(unsigned int i = 0; i < vec.size(); i++)
	{
		generate_range(vec[i], upper, i);
	}
}

template<typename result_type, typename state_type>
void Threaded_rands<result_type, state_type>::generate_range(std::vector<std::vector<result_type>>& vec, 
									const unsigned int lower, const unsigned int upper)
{
	#pragma omp parallel for num_threads(n_threads)
	for(unsigned int i = 0; i < vec.size(); i++)
	{
		generate_range(vec[i], lower, upper, i);
	}
}

// These functions are available but currently unused

// Detect the number of threads available on the machine using (if needed) multiple methods 
template<typename result_type, typename state_type>
unsigned int Threaded_rands<result_type, state_type>::get_thread_info(const int n_selected)
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
template<typename result_type, typename state_type>
unsigned int Threaded_rands<result_type, state_type>::backup_thread_count()
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
