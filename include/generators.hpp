#ifndef GENERATORS_HPP
#define GENERATORS_HPP

// Randutils by Prof. O'Neil for seeding from multiple entropy sources
#include "pcg/randutils.hpp"
#include "pcg/pcg_random.hpp"


// TODO - Add support for older version of C++

// Base class used for interfacing with any generator that has a get_rand() fn.
// template<typename state_type>
// class base_class
// {
// public:
// 	virtual state_type get_rand() = 0;
// 	virtual ~base_class(){}
// private:

// };

// A small helper class to allow the counting of instances
// of 32-bit JSF objects
// See https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
template <typename T>
class jsf_counter
{
public:
    jsf_counter()
    {
        objects_created++;
    }
    
protected:
    ~jsf_counter()
    {
        --objects_alive;
    }
    static int objects_created;
    static int objects_alive;
};
template <typename T> int jsf_counter<T>::objects_created(0);
template <typename T> int jsf_counter<T>::objects_alive(0);


// ======================================
//	 		 	SplitMix64 
// ======================================

template<typename state_type>
class splitmix64
{
protected:
	randutils::auto_seed_256 seeds;
	std::array<state_type, 2> seed_array;

	// Want an actual 64-bit int here
	std::uint64_t split_seed = 1;

public:
	splitmix64()
	{
		// Seeds from a decent entropy source
		seeds.generate(seed_array.begin(), seed_array.end());

		// Add two 32-bit ints together to create a 64-bit
		split_seed = uint64_t(seed_array[0]) << 32 | seed_array[1];
	}

	state_type operator()() { return get_rand(); }

	state_type get_rand()
	{
		state_type z = (split_seed += 0x9e3779b97f4a7c15);
		z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
		z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
		return z ^ (z >> 31);		
	}
};

// ======================================
// 		 		xoroshiro128+
// ======================================

// This is a C++ implementation of the xoroshiro128+ 32-bit PRNG
// by David Blackman and Sebastiano Vigna

// Original code available from
// http://xoshiro.di.unimi.it/

// A more general xoroshiro 128 class for 32 and 64-bit generation
template <typename state_type, std::size_t S_SIZE = 8*sizeof(state_type)>
class xoroshiro128
{
protected:
	unsigned int thread_no = 0;	
	
	const unsigned int STYPE_BITS = 8*sizeof(state_type);
	
	const std::size_t n_xoro_seeds = (STYPE_BITS == 64 ? 2 : 4);
	
	std::vector<state_type> seed_array;

	// Circular rotation function
	inline state_type rotl(const state_type x, int k) {return (x << k) | (x >> (STYPE_BITS - k));}
	
	// Create a seeded SplitMix64 instance to generate
	// further seeds for this generator.
	// This may be overkill but is only done at start
	void auto_seed()
	{
		splitmix64<state_type> seed_gen;

		for(auto &s : seed_array)
			s = seed_gen();
	}

	template <std::size_t S = S_SIZE>
	typename std::enable_if<S == 32, void>::type
	jump_stream()
	{
		const uint32_t JUMP[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

		uint32_t s0 = 0;
		uint32_t s1 = 0;
		uint32_t s2 = 0;
		uint32_t s3 = 0;

		for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		{
			for(int b = 0; b < 32; b++) 
			{
				if (JUMP[i] & UINT32_C(1) << b) {
					s0 ^= seed_array[0];
					s1 ^= seed_array[1];
					s2 ^= seed_array[2];
					s3 ^= seed_array[3];
				}
				get_rand();	
			}
		}
			
		seed_array[0] = s0;
		seed_array[1] = s1;
		seed_array[2] = s2;
		seed_array[3] = s3;
	}

	// For multiple threads - same as calling get_xoroshiro128_rand 2^64 times
	// For a 64-bit generator
	template <std::size_t S = S_SIZE>
	typename std::enable_if<S == 64, void>::type 
	jump_stream()
	{
		// static const uint64_t JUMP[] = { 0xbeac0467eba5facb, 0xd86b048b86aa9922 };
		// Updated values - 2018-10-15
		std::cout << "64-bit version\n";
		const state_type JUMP[] = { 0xdf900294d8f554a5, 0x170865df4b3201fc };

		uint64_t s0 = 0;
		uint64_t s1 = 0;

		for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		{
		for(int b = 0; b < 64; b++) 
			{
				if (JUMP[i] & UINT64_C(1) << b) 
				{
					s0 ^= seed_array[0];
					s1 ^= seed_array[1];
				}
				get_rand();
			}
		}

		seed_array[0] = s0;
		seed_array[1] = s1;
	}

	
public:
	xoroshiro128(const unsigned int thread_id) : thread_no{thread_id}
	{
		std::cout << "Creating xoroshiro128 generator for thread : " << thread_id << "\n";
		seed_array.resize(n_xoro_seeds);
		auto_seed();		
		unsigned int jump_factor = 2*thread_id;

		// Jump stream for statistically independent streams for each thread
		if(jump_factor > 0)
		{
			// std::cout << "Jumping stream for xoroshiro128+ generator on thread : " << thread_id << "\n";
			for(int x = 0; x < jump_factor; x++)
				jump_stream();
		}
	}

	// 32-bit generator
	template <std::size_t S = S_SIZE>
	typename std::enable_if<S == 32, state_type>::type
	get_rand()
	{
		const uint32_t result_plus = seed_array[0] + seed_array[3];

		const uint32_t x = seed_array[1] << 9;

		seed_array[2] ^= seed_array[0];
		seed_array[3] ^= seed_array[1];
		seed_array[1] ^= seed_array[2];
		seed_array[0] ^= seed_array[3];

		seed_array[2] ^= x;

		seed_array[3] = rotl(seed_array[3], 11);

		return result_plus;
	}
	
	// 64-bit generator
	template <std::size_t S = S_SIZE>
	typename std::enable_if<S == 64, state_type>::type
	get_rand()
	{
		const uint64_t s0 = seed_array[0];
		state_type s1 = seed_array[1];
		const uint64_t result = s0 + s1;

		// Updated the constants 24, 16, 37 here as Vigna's recommendation.
		s1 ^= s0;
		seed_array[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
		seed_array[1] = rotl(s1, 37); // c

		return result;
	}

	state_type operator()() {return get_rand(0);}

};


// ======================================
// 			pcg_unique
// ======================================

// This is a wrapper for the pcg32/64_unique
// PRNG by Prof. Melissa E. O'Neill
// http://www.pcg-random.org/

// Here we're using pcg32/64_unique for statistically independent streams

// For the PCG types
// typedef pcg_engines::unique_xsh_rr_64_32        pcg32_unique;
// typedef pcg_engines::unique_xsl_rr_128_64       pcg64_unique;
// typedef unique_base<uint32_t, uint64_t, xsh_rr_mixin>  unique_xsh_rr_64_32;
// typedef unique_base<uint64_t, pcg128_t, xsl_rr_mixin>  unique_xsl_rr_128_64;

// TODO - slim down PCG headers to extract pcg64_unique and pcg32_unique

template<typename state_type>
class pcg_unique
{
protected:	
	// typedef typename std::conditional<sizeof(state_type) == 64, pcg64_unique, pcg32_unique>::type pcg_type;
	using pcg_type = typename std::conditional<(8*sizeof(state_type) == 64), pcg64_unique, pcg32_unique>::type;

	pcg_type pcg_gen;

	unsigned int thread_no = 0;

public:
	pcg_unique(const unsigned int thread_id) : thread_no{thread_id}
	{
		std::cout << "Creating PCG generator for thread : " << thread_id << "\n";
		// Get the seeding object
		pcg_extras::seed_seq_from<std::random_device> seed_source;	

		pcg_gen = pcg_type(seed_source);
	}

	state_type get_rand() {	return pcg_gen(); }

	state_type operator()() {return get_rand();}


// // Make the class non-copyable
 //    pcg_unique(pcg_unique const&) = delete;
 //    pcg_unique& operator=(pcg_unique const&) = delete;

 //    // Make the class moveable (for placing inside the generator storage vector)
 //    pcg_unique(pcg_unique&&) = default;
 //    pcg_unique& operator=(pcg_unique&&) = default;
};


// ======================================
// 				JSF64
// ======================================

// This is an implementation of Bob Jenkins Small Fast PRNG
// Parts of this were taken from Prof. Melissa E. O'Neill's C++ implementation
// I've added seeding from a reasonable (hopefully) entropy source

// As we can't jump streams here currently this may be unsuitable for 
// highly parallel code, will try and write a jump function soon.

// Code is available here
// https://gist.github.com/imneme/85cff47d4bad8de6bdeb671f9c76c814

template<typename state_type>
class jsf : jsf_counter<state_type>
{
protected:
	
	unsigned int p, q, r;	

	unsigned int thread_no = 0;

	state_type a_, b_, c_, d_;

	// Number of bits in the state type
	const unsigned int STYPE_BITS = 8*sizeof(state_type);

	state_type rotate(state_type x, unsigned int k) { return (x << k) | (x >> (STYPE_BITS - k)); }

	// There are only 2 sets of constants for the 64-bit generator so we won't change those
	std::array<unsigned int, 3> gen_64bit_constants = {7, 13, 37};
	// These can be used to create statistically different streams for the 32-bit generator

	std::vector<std::vector<unsigned int>> gen_32bit_constants =  {{3, 14, 24},
									{3, 25, 15},
									{4, 15, 24},
									{6, 16, 28},
									{7, 16, 27},
									{8, 14,  3},
									{11, 16, 23},
									{12, 16, 22},
									{12, 17, 23},
									{13, 16, 22},
									{15, 25,  3},
									{16,  9,  3},
									{17,  9,  3},
									{17, 27,  7},
									{19,  7,  3},
									{23, 15, 11},
									{23, 16, 11},
									{23, 17, 11},
									{24,  3, 16},
									{24,  4, 16},
									{25, 14,  3},
									{27, 16,  6},
									{27, 16,  7}};


public:
	jsf(const unsigned int thread_id) : thread_no{thread_id}, a_{0xf1ea5eed}
	{
		std::cout << "Creating JSF generator for thread id : " << thread_id << " \n";

		splitmix64<state_type> seed_gen;

		// Seed the generator using the SplitMix64 object
		seed(seed_gen());

		// Keep track of how many instances of the generator
		// we have for the constants
		if(STYPE_BITS == 32)
		{
			// Although this will also count 64-bit instances it
			// shouldn't matter for small amounts of objects
			unsigned int jsf32_instances = jsf_counter<state_type>::objects_created;

			// Roll over if we have more than 23 instances
			unsigned int jsf_constants = (jsf32_instances > 22) ? 0 : jsf32_instances;

			// For zero-order array access
			if(jsf_constants > 0)
				jsf_constants -= 1;

			p = gen_32bit_constants[jsf_constants][0];
			q = gen_32bit_constants[jsf_constants][1];
			r = gen_32bit_constants[jsf_constants][2];
		}
		if(STYPE_BITS == 64)
		{			
			p = 7;
			q = 13;
			r = 37;
		}
	}

	void seed(const state_type seed)
    {
    	b_ = seed;
    	c_ = seed;
    	d_ = seed;

    	for(unsigned int i=0; i < 20; i++)
            advance();
    }

    void advance()
    {
        state_type e = a_ - rotate(b_, p);
        a_ = b_ ^ rotate(c_, q);
        b_ = c_ + (r ? rotate(d_, r) : d_);
        c_ = d_ + e;
        d_ = e + a_;
    }

    state_type get_rand()
    {
    	advance();
        return d_;
    }

    state_type operator()() { return get_rand(); }

};

#endif