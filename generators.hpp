#ifndef GENERATORS_HPP
#define GENERATORS_HPP

#include "threaded_rands.hpp"
// Randutils by Prof. O'Neil for seeding from multiple entropy sources
#include "randutils.hpp"
// TODO - slim down the PCG library, extract pcg64_unique for use
#include "pcg/pcg_random.hpp"
// TODO - finish modifying this for 32-bit threaded use
#include "jsf_implementation.hpp"

// Base class used for interfacing with any generator that has a get_rand() fn.
class base_class
{
public:
	virtual uint64_t get_rand() = 0;
	virtual ~base_class(){}
private:

};

// ======================================
// 		 		xoroshiro128+
// ======================================

// This is a C++ implementation of the xoroshiro128+ PRNG
// by David Blackman and Sebastiano Vigna

// Original code available from
// http://xoshiro.di.unimi.it/

class xoroshiro128 : public base_class
{
public:
	xoroshiro128(const int thread_id) : thread_no(thread_id)
	{
		auto_seed();		
		unsigned int jump_factor = 2*thread_id;

		// Jump stream for statistically independent streams for each thread
		if(jump_factor > 0)
		{
			std::cout << "Jumping stream for xoroshiro128+ generator on thread : " << thread_id << "\n";
			for(int x = 0; x < jump_factor; x++)
				jump_stream();
		}
	}

	~xoroshiro128() {}
	
	uint64_t operator()() {return get_rand();}

	uint64_t get_rand() override;
private:

	// For multiple threads - same as calling get_xoroshiro128_rand 2^64 times
	void jump_stream();
	// Uses the auto_seed_256 function from rand_utils to seed SplitMix64 and fills the seed_array
	void auto_seed();

	uint64_t splitmix_seeder(const uint64_t x);	

	std::size_t n_rands = 1;

	// This keeps track of the thread number, for each xoroshiro thread 
	// jump * thread number is used to ensure individual threads
	int thread_no = 0;
	
	static const std::size_t n_xoro_seeds = 2;

	std::array<uint64_t, n_xoro_seeds> seed_array;

	// Original xoroshiro code
	static inline uint64_t rotl(const uint64_t x, int k) {return (x << k) | (x >> (64 - k));}

};

void xoroshiro128::auto_seed()
{
	// Get the seed value
	randutils::auto_seed_256 seeds;	

	// This doesn't return a uint64_t but it should be OK?
	seeds.generate(seed_array.begin(), seed_array.end());	
	
	// Update the seed values with SplitMix64 generated values
	for(auto& x : seed_array)
		x = splitmix_seeder(x);
}

uint64_t xoroshiro128::splitmix_seeder(uint64_t x)
{
	// uint64_t x;  The state can be seeded with any value. 

	uint64_t z = (x += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

// This is the xoroshiro128+ PRNG
uint64_t xoroshiro128::get_rand()
{
	const uint64_t s0 = seed_array[0];
	uint64_t s1 = seed_array[1];
	const uint64_t result = s0 + s1;


	// Updated the constants 24, 16, 37 here as Vigna's recommendation.
	s1 ^= s0;
	seed_array[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	seed_array[1] = rotl(s1, 37); // c

	return result;
}

// For multiple threads
void xoroshiro128::jump_stream()
{	
	// static const uint64_t JUMP[] = { 0xbeac0467eba5facb, 0xd86b048b86aa9922 };
	// Updated values - 2018-10-15
	static const uint64_t JUMP[] = { 0xdf900294d8f554a5, 0x170865df4b3201fc };

	uint64_t s0 = 0;
	uint64_t s1 = 0;

	for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		for(int b = 0; b < 64; b++) 
		{
			if (JUMP[i] & UINT64_C(1) << b) 
			{
				s0 ^= seed_array[0];
				s1 ^= seed_array[1];
			}
			get_rand();
		}

	seed_array[0] = s0;
	seed_array[1] = s1;
}


// ======================================
// 			pcg64_unique
// ======================================

// This is a wrapper for the pcg64_unique
// PRNG by Prof. Melissa E. O'Neill
// http://www.pcg-random.org/

// Here we're using pcg64_unique for (hopefully) statistically independent streams

// TODO - slim down PCG headers to extract pcg64_unique and pcg32_unique
class pcg64_wrap : public base_class
{
public:
	pcg64_wrap(const unsigned int thread_id) : pcg64_gen(seed_source)
	{
		std::cout << "Creating pcg64_unique generator for thread : " << thread_id << "\n";
	}

	~pcg64_wrap() {}

	uint64_t operator()() {return pcg64_gen();}

	uint64_t get_rand() override {return pcg64_gen();}

private:

	std::size_t n_rands = 1;
	
	pcg_extras::seed_seq_from<std::random_device> seed_source;	
    // Use a pcg64_unique as then we have a unique stream for each instance
	pcg64_unique pcg64_gen;
	
};


// ======================================
// 				JSF64
// ======================================

// This is an implementation of Bob Jenkins Small Fast PRNG
// Parts of this were taken from Prof. Melissa E. O'Neill's C++ implementation
// I've added seeding from a reasonable (hopefully) entropy source

// As we can't jump streams here currently this may be unsuitable for 
// highly parallel code, will try and write a jump function soon.

// Her code is available here
// https://gist.github.com/imneme/85cff47d4bad8de6bdeb671f9c76c814

// TODO  - implement constant variations available in Prof O'Neil's code
// in template instantiation.

class jsf64_wrap : public base_class
{
public:
	jsf64_wrap(const unsigned int thread_id) : thread_no(thread_id)
	{
		std::cout << "Creating JSF64 generator for thread id : \n";

		std::vector<uint64_t> seed_vec(2);
		randutils::auto_seed_256 seed_source;
		seed_source.generate(seed_vec.begin(), seed_vec.end());

		// Could just use SplitMix here but hey - this is more fun
		// Here we'll use 2 32-bit integers to create a 64-bit int to use as a seed
		std::uint32_t part_one = seed_vec.at(0);
		std::uint32_t part_two = seed_vec.at(1);
		// Shift a 32-bit int over by 32 bits and add the second 32-bit int on the end
		uint64_t seed64 = uint64_t(part_one) << 32 | part_two;

		jsf_gen.seed(seed64);
	}

	~jsf64_wrap() {}

	uint64_t operator()() {return jsf_gen();}

	uint64_t get_rand() override { return jsf_gen(); }
	
private:
	// JSF64 Generator object
	jsf64 jsf_gen;	

	int thread_no = 1;

};

#endif