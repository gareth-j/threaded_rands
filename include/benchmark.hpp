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
#include <immintrin.h>

#ifdef _MSC_VER
# include <intrin.h>
#else
# include <x86intrin.h>
#endif

#include "system_seed.hpp"


#include "pcg32.hpp"
#include "pcg64.hpp"


#ifndef __x86_64__
#warning "Expecting an x64 processor."
#endif

class benchmark
{
private:
	pcg32 my_pcg32;
	pcg64 my_pcg64;

	// Number of random numbers to generate
	const std::size_t N_rands = 4*65536;//50000;
	
	// The number of  random numbers used in the shuffling
	// benchmark
	const std::size_t N_shuffle = 10000;
	
	// The number of times to repeat each function benchmark
	const std::size_t repeats = 500;
	
	// Use a vector here in case a lot of rands are requested
	std::vector<uint32_t> rand_arr32;
	std::vector<uint64_t> rand_arr64;

	// Benchmarking functions
	// These are taken from Lemire's testrandom 
    void RDTSC_start(uint64_t* cycles)
    {
        // (Hopefully) place these in a register
        register unsigned cyc_high, cyc_low;

        // Use the read time-stamp counter so we can count the 
        // number of cycles
        __asm volatile("cpuid\n\t"                                               
                       "rdtsc\n\t"                                               
                       "mov %%edx, %0\n\t"                                       
                       "mov %%eax, %1\n\t"                                       
                       : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx",  
                         "%rdx");    

        // Update the number of cycles
        *cycles = (uint64_t(cyc_high) << 32) | cyc_low; 
    }

    void RDTSC_final(uint64_t* cycles)
    {
        // (Hopefully) place these in a register
        register unsigned cyc_high, cyc_low;

        // Use the read time-stamp counter so we can count the 
        // number of cycles
        __asm volatile("rdtscp\n\t"                                               
                       "mov %%edx, %0\n\t"
                       "mov %%eax, %1\n\t"
                       "cpuid\n\t"
                       : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx",
                         "%rdx");

        *cycles = (uint64_t(cyc_high) << 32) | cyc_low;                          
    }

	inline uint64_t readTSC() 
	{
	    // _mm_lfence();  // optionally wait for earlier insns to retire before reading the clock
	    uint64_t tsc = __rdtsc();
	    // _mm_lfence();  // optionally block later instructions until rdtsc retires
	    return tsc;
	}


    // template <typename RTYPE, typename TEST_CLASS>
    // void fill_array(RTYPE (TEST_CLASS::*rand_fn)(), TEST_CLASS& class_obj)

    // This function can be passed a member function and an array for testing 
    template <typename TEST_CLASS, typename RTYPE>
    void benchmark_fn(void (TEST_CLASS::*test_fn)(RTYPE*, size_t), TEST_CLASS& class_obj, RTYPE* test_array, const std::size_t size, const std::string& str)
    {
    	std::fflush(nullptr);

		// printf("Generating %d bytes of random numbers \n", size);
		// printf("Time reported in number of cycles per byte.\n");
		// printf("We store values to an array of size = %d kB.\n", size / (1024));
		// assert(size / 8 * 8 == size);

        uint64_t cycles_start{0}, cycles_final{0}, cycles_diff{0};

        uint64_t my_cycles_start{0}, my_cycles_end{0}, my_cycles_diff{0};

        // Get max value of a uin64_t by rolling it over
        uint64_t min_diff = (uint64_t)-1;
        uint64_t my_min_diff = (uint64_t)-1;

        std::cout << "Testing function : " << str << "\n"; 

        for(uint32_t i = 0; i < repeats; i++)
        {
            // Pretend to clobber memory 
            // Don't allow reordering of memory access instructions
            __asm volatile("" ::: "memory");
            
            RDTSC_start(&cycles_start);

            my_cycles_start = readTSC();
         
            // (class_obj.*test_fn)(test_array, size);
            (class_obj.*test_fn)(test_array, size); // Sort me out!
            
            RDTSC_final(&cycles_final);

            my_cycles_end = readTSC();

            cycles_diff = (cycles_final - cycles_start);   
            my_cycles_diff = my_cycles_end - my_cycles_start;
            
            if (cycles_diff < min_diff)
                min_diff = cycles_diff;

            if (my_cycles_diff < my_min_diff)
                my_min_diff = my_cycles_diff;
        }  

        // Number of bytes to be processed
        uint64_t S = size;

        // Calculate the number of cycles per operation
        float cycles_per_op = min_diff / float(S);
        float my_cycles_per_op = my_min_diff / float(S);

        std::cout << std::setprecision(2) << cycles_per_op << " cycles per byte\n";
        std::cout << std::setprecision(2) << my_cycles_per_op << " my cycles per byte\n";
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
		std::string fn_name = "pcg64_c";
		benchmark_fn(&pcg64::fill_array, my_pcg64, rand_arr64.data(), N_rands, fn_name);

		fn_name = "populate_array_pcg32";
    	benchmark_fn(&pcg32::populate_array_pcg32, my_pcg32, rand_arr32.data(), N_rands, fn_name);
	}

    
};

#endif