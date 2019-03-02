// Tiny self-contained version of the PCG Random Number Generation for C++
// put together from pieces of the much larger C/C++ codebase.
// Wenzel Jakob, February 2015
// 
// The PCG random number generator was developed by Melissa O'Neill
// <oneill@pcg-random.org>
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// For additional information about the PCG random number generation scheme,
// including its license and other licensing options, visit
// 
//     http://www.pcg-random.org
 
#ifndef PCG32_H
#define PCG32_H

#include <cstdint>
#include <cmath>
#include <cassert>
#include <algorithm>

// #include "system_seed.hpp"

// PCG32 Pseudorandom number generator
// This version doesn't use the key class
class pcg32
{
private:
	// RNG state.  All values are possible.
    uint64_t state;
    // Controls which RNG sequence (stream) is selected. Must *always* be odd.
    uint64_t inc;

    // Constants - taken from O'Neil's C version of PCG
    const uint64_t PCG_DEFAULT_MULTIPLIER_64 = 6364136223846793005ULL;
    const uint64_t PCG_DEFAULT_INCREMENT_64 = 1442695040888963407ULL;
  
public:
    // Seed the generator with system generated random numbers
    pcg32()
    {
    	std::array<uint32_t, 4> seed_array;
	    system_seed seeder;		
		seeder.generate(seed_array.begin(), seed_array.end());

		// Create some 64-bit numbers
		uint64_t seed = (static_cast<uint64_t>(seed_array[0]) << 32) | seed_array[1];
		uint64_t initseq = (static_cast<uint64_t>(seed_array[2]) << 32) | seed_array[3];

		// The same seeding process used in the O'Neill's C code
		// and ensures inc is odd
		state = 0U;
		inc = (initseq << 1u) | 1u;
		get_rand();
		state += seed;
		get_rand();
    }

    pcg32(const uint32_t seed1, const uint32_t seed2)
    {
    	uint64_t seed = uint64_t(seed1) << 32 | seed2;
    	uint64_t initseq = PCG_DEFAULT_INCREMENT_64;

    	state = 0U;
		inc = (initseq << 1u) | 1u;
		get_rand();
		state += seed;
		get_rand();
    }
    
    void populate_array(uint32_t* rand_arr, const size_t size)
    {
        for(uint32_t i = 0; i < size; ++i)
        {
            rand_arr[i] = get_rand();
        }
    }

    uint32_t operator()()
    {
    	return get_rand();
    }

    // Generate a uniformly distributed unsigned 32-bit random number
    uint32_t get_rand() 
    {
        uint64_t oldstate = state;
        state = oldstate * PCG_DEFAULT_MULTIPLIER_64 + inc;
        uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        uint32_t rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
    }
};

#endif // 	PCG32_H
