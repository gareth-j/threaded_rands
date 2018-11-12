#include "include/threaded_rands.hpp"

int main()
{
<<<<<<< HEAD
	// Select the types we want to use
	using result_type = std::uint64_t;
	using state_type = std::uint64_t;

	// Select the generator
	const generator_type selection = generator_type::xoro128;

	// The number of random numbers we want to generate
	const size_t n_rands = 1e8;

	// How many threads to use
	const unsigned int n_threads = 8;
=======
	// Select the generator
	// Available generators are the xoro128, pcg and jsf PRNGs
	const generator_type selection = generator_type::jsf;

	// How many random numbers to generate
	const size_t n_rands = 1e8; // or 100'000'000

	// How many threads to use
	const unsigned int n_threads = 8;

	// The type we want returned
	// Currently only integer types
	using rand_type = std::uint64_t;
	using state_type = std::uint64_t;
	
	// Lets make some big-ish vectors	
	std::vector<std::vector<rand_type>> vector_storage;
	for(int i = 0; i < n_threads; i++)
		vector_storage.emplace_back(std::vector<rand_type>(n_rands));

	// Create the PRNG 
	Threaded_rands<rand_type, state_type> my_generator(n_threads, selection);

	// Use a high resolution clock from the standard library for timing
	using hr_clock = std::chrono::high_resolution_clock;
	
	std::cout << "\nFilling a vector with random numbers...\n";

	auto t_start  = hr_clock::now();

	my_generator.generate_2D(vector_storage);
	
	auto t_end = hr_clock::now();
>>>>>>> devel

	// Somewhere to store them
	std::vector<std::uint64_t> rand_vec(n_rands);

<<<<<<< HEAD
	// Instantiate the Threaded_rands object
	Threaded_rands<result_type, state_type> my_generator(selection, n_threads);
=======
	std::cout << "\nIt took " << duration.count() << " ms to fill " << n_threads << " vectors of " << n_rands << " rands.\n";
	
	std::cout << "\nA sample of random numbers : \n\n";
	
	for(unsigned int i = 0; i < 8; ++i)
	{
		std::cout << "An integer : " << my_generator()  << " and a double : " << my_generator.get_double() << std::endl;		
	}

	std::cout << "\nFinished generating.\n";
>>>>>>> devel

	// Fill the vector
	my_generator.generate(vector_storage);
}