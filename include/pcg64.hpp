#ifndef MY_PCG64_H
#define MY_PCG64_H

/* Modified by D. Lemire based on original code by M. O'Neill, August 2017 */
// #include <stdint.h>
// #include "splitmix64.h" // we are going to leverage splitmix64 to generate the seed

#include <iostream>

class pcg64
{
private:
	using pcg128_t = __uint128_t;

	pcg128_t state;
	pcg128_t inc;

	const pcg128_t PCG_DEFAULT_MULTIPLIER_128 = (static_cast<pcg128_t>(2549297995355413924ULL) << 64) + 4865540595714422341ULL; 
	// This is currently unused as we set the increment during seeding
	const pcg128_t PCG_DEFAULT_INCREMENT_128 = (static_cast<pcg128_t>(6364136223846793005ULL) << 64) + 1442695040888963407ULL;

	inline uint64_t get_rand()
	{
		return pcg_setseq_128_xsl_rr_64_random_r();
	}

	inline void pcg_setseq_128_step_r()
	{
		state = state * PCG_DEFAULT_MULTIPLIER_128 + inc;
	}

	// verbatim from O'Neill's except that we skip her assembly:
	inline uint64_t pcg_rotr_64(uint64_t value, unsigned int rot) 
	{
		return (value >> rot) | (value << ((-rot) & 63));
	}

	inline uint64_t pcg_output_xsl_rr_128_64() 
	{
		return pcg_rotr_64(((uint64_t)(state >> 64u)) ^ (uint64_t)state, state >> 122u);
	}

	// This is the get_rand function really
	inline uint64_t pcg_setseq_128_xsl_rr_64_random_r() 
	{
		pcg_setseq_128_step_r();
		return pcg_output_xsl_rr_128_64();
	}

public:
	// Automatically seed the object from system generated rands
	pcg64()
	{
		system_seed seeder;
		std::array<uint64_t, 4> seed_arr;		
		seeder.generate(seed_arr.begin(), seed_arr.end());

		for(auto x : seed_arr)
			std::cout << x << "\n";

		uint64_t part1 = seed_arr[0];
		uint64_t part2 = seed_arr[1];
		uint64_t part3 = seed_arr[2];
		uint64_t part4 = seed_arr[3];

		pcg128_t seed = (static_cast<pcg128_t>(part1) << 64) | part2;
		pcg128_t initseq = (static_cast<pcg128_t>(part3) << 64) | part4;

		state = 0U;
		inc = (initseq << 1u) | 1u;
		get_rand();
		state += seed;
		get_rand();
	}

	void fill_array(uint64_t* rand_arr, const size_t N_rands)
	{
		for(size_t i = 0; i < N_rands; ++i)
			rand_arr[i] = pcg_setseq_128_xsl_rr_64_random_r();
	}

};

// typedef __uint128_t pcg128_t;
// #define PCG_128BIT_CONSTANT(high, low) ((((pcg128_t)high) << 64) + low)
// #define PCG_DEFAULT_MULTIPLIER_128                                             \
//   PCG_128BIT_CONSTANT(2549297995355413924ULL, 4865540595714422341ULL)
// #define PCG_DEFAULT_INCREMENT_128                                              \
//   PCG_128BIT_CONSTANT(6364136223846793005ULL, 1442695040888963407ULL)

// struct pcg_state_setseq_128 {
//   pcg128_t state;
//   pcg128_t inc;
// };

// typedef struct pcg_state_setseq_128 pcg64_random_t;

// static inline void pcg_setseq_128_step_r(struct pcg_state_setseq_128 *rng) {
//   rng->state = rng->state * PCG_DEFAULT_MULTIPLIER_128 + rng->inc;
// }

// static inline void pcg_setseq_128_srandom_r(struct pcg_state_setseq_128 *rng,
//                                             pcg128_t initstate,
//                                             pcg128_t initseq) {
//   rng->state = 0U;
//   rng->inc = (initseq << 1u) | 1u;
//   pcg_setseq_128_step_r(rng);
//   rng->state += initstate;
//   pcg_setseq_128_step_r(rng);
// }

// // verbatim from O'Neill's except that we skip her assembly:
// static inline uint64_t pcg_rotr_64(uint64_t value, unsigned int rot) 
// {
//   return (value >> rot) | (value << ((-rot) & 63));
// }

// static inline uint64_t pcg_output_xsl_rr_128_64(pcg128_t state) 
// {
//   return pcg_rotr_64(((uint64_t)(state >> 64u)) ^ (uint64_t)state,
//                      state >> 122u);
// }

// static inline uint64_t
// pcg_setseq_128_xsl_rr_64_random_r(struct pcg_state_setseq_128 *rng) {
//   pcg_setseq_128_step_r(rng);
//   return pcg_output_xsl_rr_128_64(rng->state);
// }

// /***
// * rest is our code
// ****/

// // use use a global state:
// static pcg64_random_t pcg64_global; // global state

// // call this once before calling pcg64_random_r
// static inline void pcg64_seed(uint64_t seed) {
//   pcg128_t initstate = PCG_128BIT_CONSTANT(splitmix64_stateless(seed),
//                                            splitmix64_stateless(seed + 1));
//   // we pick a sequence at random
//   pcg128_t initseq = PCG_128BIT_CONSTANT(splitmix64_stateless(seed + 2),
//                                          splitmix64_stateless(seed + 3));
//   initseq |= 1; // should not be necessary, but let us be careful.

//   pcg_setseq_128_srandom_r(&pcg64_global, initstate, initseq);
// }

// #define pcg64_random_r pcg_setseq_128_xsl_rr_64_random_r

// static inline uint64_t pcg64(void) { return pcg64_random_r(&pcg64_global); }

#endif
