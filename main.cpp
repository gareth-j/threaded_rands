#include "threaded_rands.hpp"

int main()
{
	// Select the types we want to use
	using result_type = std::uint64_t;
	using state_type = std::uint64_t;

	// Select the generator
	const generator_type selection = generator_type::xoro128;

	// The number of random numbers we want to generate
	const size_t n_rands = 1e8;

	// How many threads to use
	const unsigned int n_threads = 8;

	// Somewhere to store them
	std::vector<std::uint64_t> rand_vec(n_rands);

	// Instantiate the Threaded_rands object
	Threaded_rands<result_type, state_type> my_generator(selection, n_threads);

	// Fill the vector
	my_generator.generate(vector_storage);
}