#ifndef PCG64_H
#define PCG64_H

#include <iostream>

// Check we have 128-bit ints
#ifndef __SIZEOF_INT128__
    #error 128-bit integer type required for 64-bit PCG generator
#endif

class pcg64
{
private:
	using pcg128_t = __uint128_t;

	pcg128_t state;
	pcg128_t inc;

	// Multiplication constant
	const pcg128_t PCG_DEFAULT_MULTIPLIER_128 = (static_cast<pcg128_t>(2549297995355413924ULL) << 64) + 4865540595714422341ULL; 
	// This is currently unused as we set the increment during seeding
	const pcg128_t PCG_DEFAULT_INCREMENT_128 = (static_cast<pcg128_t>(6364136223846793005ULL) << 64) + 1442695040888963407ULL;

	// inline void pcg_setseq_128_step_r()
	// {
	// 	state = state * PCG_DEFAULT_MULTIPLIER_128 + inc;
	// }

	// // verbatim from O'Neill's except that we skip her assembly:
	// inline uint64_t pcg_rotr_64(uint64_t value, unsigned int rot) 
	// {
	// 	return (value >> rot) | (value << ((-rot) & 63));
	// }

	// inline uint64_t pcg_output_xsl_rr_128_64() 
	// {
	// 	return pcg_rotr_64(((uint64_t)(state >> 64u)) ^ (uint64_t)state, state >> 122u);
	// }

	// This is the get_rand function really
	inline uint64_t pcg_setseq_128_xsl_rr_64_random_r() 
	{
		state = state * PCG_DEFAULT_MULTIPLIER_128 + inc;
		uint64_t value = (uint64_t)(state >> 64u) ^ (uint64_t)state;
		uint64_t rot = state >> 122u;

		return (value >> rot) | (value << ((-rot) & 63));
	}

public:
	// Seed the generator with system generated random numbers
	pcg64()
	{
		std::array<uint64_t, 4> seed_arr;
		system_seed seeder;
		seeder.generate(seed_arr.begin(), seed_arr.end());

		pcg128_t seed = (static_cast<pcg128_t>(seed_arr[0]) << 64) | seed_arr[1];
		pcg128_t initseq = (static_cast<pcg128_t>(seed_arr[2]) << 64) | seed_arr[3];

		state = 0U;
		inc = (initseq << 1u) | 1u;
		get_rand();
		state += seed;
		get_rand();
	}

	pcg64(const uint64_t seed1, const uint64_t seed2)
	{
		pcg128_t seed = static_cast<pcg128_t>(seed1) << 64 | seed2;
		pcg128_t initseq = PCG_DEFAULT_INCREMENT_128;

		state = 0U;
		inc = (initseq << 1u) | 1u;
		get_rand();
		state += seed;
		get_rand();
	}

	inline uint64_t get_rand()
	{
		return pcg_setseq_128_xsl_rr_64_random_r();
	}

	inline uint64_t operator()()
	{
		return pcg_setseq_128_xsl_rr_64_random_r();
	}

	void populate_array(uint64_t* rand_arr, const size_t N_rands)
	{
		for(size_t i = 0; i < N_rands; ++i)
			rand_arr[i] = get_rand();
	}
};

#endif // PCG64_H
