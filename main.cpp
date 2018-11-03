#include "threaded_rands.hpp"

int main()
{
	const generator_type selection = generator_type::pcg64;

	const size_t n_rands = 1e8;
	const unsigned int n_threads = 8;

	using rand_type = int;
	using state_type = std::uint64_t;

	// A big vector of vectors
	std::vector<rand_type> rand_vec(n_rands);
	std::vector<std::vector<rand_type>> vector_storage;

	// Lets make some big-ish vectors	
	for(int i = 0; i < n_threads; i++)
		vector_storage.emplace_back(std::vector<rand_type>(n_rands));

	// Threaded_rands<result_type, state_type>
	Threaded_rands<rand_type, rand_type> my_generator(n_threads);

	// How long does it take
	using hr_clock = std::chrono::high_resolution_clock;
	
	auto t_start  = hr_clock::now();

	// my_generator.generate_2D(vector_storage);
	
	auto t_end = hr_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start);

	// std::cout << "A sample of random numbers : ";
	// for(int i = 0; i < 5; ++i)
	// 	std::cout << vector_storage.at(0).at(i) << " ";
	// std::cout << "\n";

	std::cout << duration.count() << " ms to fill " << n_threads << " vectors of " << n_rands << " rands with the pcg64 generator.\n";

} // End main