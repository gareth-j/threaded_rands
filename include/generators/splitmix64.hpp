
// Modified by Gareth Jones, April 2019
// to include automatic seeding

/* Modified by D. Lemire, August 2017 */
/***
Fast Splittable Pseudorandom Number Generators
Steele Jr, Guy L., Doug Lea, and Christine H. Flood. "Fast splittable pseudorandom number generators." 
ACM SIGPLAN Notices 49.10 (2014): 453-472.
***/

/*  Written in 2015 by Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

// original documentation by Vigna:
/* This is a fixed-increment version of Java 8's SplittableRandom generator
   See http://dx.doi.org/10.1145/2714064.2660195 and
   http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html

   It is a very fast generator passing BigCrush, and it can be useful if
   for some reason you absolutely want 64 bits of state; otherwise, we
   rather suggest to use a xoroshiro128+ (for moderately parallel
   computations) or xorshift1024* (for massively parallel computations)
   generator. */

#ifndef SPLITMIX64_H
#define SPLITMIX64_H

class splitmix64
{
private:
	uint64_t splitmix64_state;

public:
	splitmix64()
	{
    	std::array<uint32_t, 2> seed_array;
	    system_seed seeder;
		seeder.generate(seed_array.begin(), seed_array.end());

		splitmix64_state = (static_cast<uint64_t>(seed_array[0]) << 32) | seed_array[1];
	}

	// Allow setting of the seed
	splitmix64(const uint64_t seed) : splitmix64_state{seed}
	{} // Nothing to do here

	inline uint64_t get_rand()
	{
		uint64_t z = (splitmix64_state += UINT64_C(0x9E3779B97F4A7C15));
		z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
		z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
		return z ^ (z >> 31);
	}

	uint64_t operator()()
	{
		return get_rand();
	}

	void populate_array(uint64_t* rand_arr, const size_t N_rands)
	{
		for(size_t i = 0; i < N_rands; ++i)
			rand_arr[i] = get_rand();
	}
};

#endif // SPLITMIX64_H
