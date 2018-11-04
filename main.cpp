#include "threaded_rands.hpp"

int main()
{
	const generator_type selection = generator_type::xoro128;

	const size_t n_rands = 1e8;
	const unsigned int n_threads = 8;

	using result_type = std::uint64_t;
	using state_type = std::uint64_t;

	// A big vector of vectors
	std::vector<std::vector<uint64_t>> vector_storage;

	// Lets make some big-ish vectors	
	for(int i = 0; i < n_threads; i++)
		vector_storage.emplace_back(std::vector<uint64_t>(n_rands));

	Threaded_rands<result_type, state_type> my_generator(selection, n_threads);

	// How long does it take
	using hr_clock = std::chrono::high_resolution_clock;
	
	auto t_start  = hr_clock::now();

	my_generator.generate_2D(vector_storage);

	auto t_end = hr_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start);

	std::cout << duration.count() << " ms to fill " << n_threads << " vectors of " << n_rands << " rands with the xoro128 generator.\n";

} // End main