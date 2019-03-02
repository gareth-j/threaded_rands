// Threaded_rands - get statistically good random numbers from multiple threads
// Copyright (C) 2019 Gareth Jones

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <cstring>
#include <iostream>
#include <array>
#include <random>
#include <iomanip>
#include <functional>
#include <immintrin.h>

// Check for MS compiler
#ifdef _MSC_VER
# include <intrin.h>
#else
# include <x86intrin.h>
#endif

// For seeding from the system
// random number generator
#include "system_seed.hpp"


#include "pcg32.hpp"
#include "pcg64.hpp"
#include "splitmix64.hpp"
#include "lehmer64.hpp"

#ifndef __x86_64__
#warning "Expecting an x64 processor."
#endif

class benchmark
{
private:
	pcg32 pcg32_gen;
	pcg64 pcg64_gen;
	splitmix64 splitmix64_gen;
	lehmer64 lehmer64_gen;

	// Number of random numbers to generate
	// Here we have 256 kB (4*64kB) of random numbers
	const std::size_t N_rands = 4*65536;
	
	// The number of  random numbers used in the shuffling
	// benchmark
	const std::size_t N_shuffle = 10000;
	
	// The number of times to repeat each function benchmark
	const std::size_t repeats = 500;
	
	// Use a vector here in case a lot of rands are requested
	std::vector<uint32_t> rand_arr32;
	std::vector<uint64_t> rand_arr64;

	// Read from the Time Stamp Counter register
	inline uint64_t read_TSC() 
	{
	    // _mm_lfence();  // optionally wait for earlier insns to retire before reading the clock
	    uint64_t tsc = __rdtsc();
	    // _mm_lfence();  // optionally block later instructions until rdtsc retires
	    return tsc;
	}

	 // This function can be passed a member function and an array for testing 
    template <typename TEST_CLASS, typename RTYPE>
    void benchmark_fn(void (TEST_CLASS::*test_fn)(RTYPE*, size_t), TEST_CLASS& class_obj, RTYPE* test_array, const std::size_t size, const std::string& str)
    {
    	std::fflush(nullptr);

        uint64_t cycles_start{0}, cycles_end{0}, cycles_diff{0};

        // Get max value of a uin64_t by rolling it over
        uint64_t min_diff = (uint64_t)-1;

        std::cout << "Testing function : " << str << "\n"; 

        for(uint32_t i = 0; i < repeats; i++)
        {
            // Pretend to clobber memory 
            // Don't allow reordering of memory access instructions
            __asm volatile("" ::: "memory");
            
            cycles_start = read_TSC();
         
            (class_obj.*test_fn)(test_array, size);

            cycles_end = read_TSC();
			
			cycles_diff = cycles_end - cycles_start;
            
            if (cycles_diff < min_diff)
                min_diff = cycles_diff;
        }  

        // How many bytes we've generated
        uint64_t N_bytes = size * sizeof(RTYPE);

        // Calculate the number of cycles per operation
        // float cycles_per_op = min_diff / float(S);
        double cycles_per_byte = min_diff / double(N_bytes);

        std::cout << std::setprecision(2) << cycles_per_byte << " my cycles per byte\n";

        std::fflush(nullptr);
    }

public:
	benchmark() 
	{
		rand_arr32.resize(N_rands);
		rand_arr64.resize(N_rands);
	}

	void benchmark_generators()
	{
		std::cout << "\nBenchmarking generators....\n";

    	std::string fn_name = "pcg32 - populate_array";    	
    	benchmark_fn(&pcg32::populate_array, pcg32_gen, rand_arr32.data(), N_rands, fn_name);

		fn_name = "pcg64 - populate_array";		
		benchmark_fn(&pcg64::populate_array, pcg64_gen, rand_arr64.data(), N_rands, fn_name);

		fn_name = "lehmer64 - populate_array";		
		benchmark_fn(&lehmer64::populate_array, lehmer64_gen, rand_arr64.data(), N_rands, fn_name);

		fn_name = "splitmix64 - populate_array";		
		benchmark_fn(&splitmix64::populate_array, splitmix64_gen, rand_arr64.data(), N_rands, fn_name);
	}

    
};

#endif