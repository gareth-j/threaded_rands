#ifndef LEHMER_64
#define LEHMER_64

class lehmer64
{
private:
	__uint128_t lehmer64_state;
	const unsigned long long multiplier = 0xda942042e4dd58b5ULL;

public:
	// By default seed with system entropy
	lehmer64()
	{
		std::array<uint32_t, 2> seed_array;
		system_seed seeder;
		seeder.generate(seed_array.begin(), seed_array.end());

		lehmer64_state = (static_cast<__uint128_t>(seed_array[0]) << 64) | seed_array[1];
		// Ensure the state is odd
		lehmer64_state |= 1;
	}

	// Allow manual setting of the seed
	lehmer64(const uint64_t seed1, const uint64_t seed2)
	{
		lehmer64_state = (static_cast<__uint128_t>(seed1) << 64) | seed2;
	}

	uint64_t get_rand()
	{
		lehmer64_state *= multiplier;

		return lehmer64_state >> 64;
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

#endif // end LEHMER_64