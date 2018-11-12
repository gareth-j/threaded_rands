# Threaded_rands

A library to produce statistically unique streams of psuedo-random
numbers (PRNs) in parallel written in modern C++

## Getting Started

There are three generator types to select from

1. pcg64 - an implementation of the PCG64 generator by Prof. Melissa O'Neill, 
 	   a statistically good algorithm for creating pseudo-random numbers in parallel,
	   uses the pcg64_unique from her PCG library

2. xoro128 - an implementation of the xoroshiro128+ PRNG from Sebastian Vigna, another statistically good
	     generator that can be used with multiple threads

3. jsf64   - an implementation of the JSF PRNG by Bob Jenkins with good statistical properties. 
             Still under testing for larger numbers of threads.

### Usage
```
#include "threaded_rands.hpp"
```

Create a threaded_rands object and (if you want) select the generator and number of threads

For a default generator using 1 thread and the pcg64_unique PRNG

```
Threaded_rands<uint64_t, uint64_t> my_generator();
```

If you just want random numbers that are from a statistically good algorithm and
are seeding with a good entropy source just use

```
my_generator();
```

which returns a uint64_t from the generator.

Create a generator using 8 threads and the xoroshiro128+ PRNG

```
generator_type my_selection = generator_type::xoro128;

Threaded_rands<uint64_t, uint64_t> my_generator(my_selection, 8);
```

Then you can pass an 1D or 2D array of a type you want

```
std::vector<uint64_t> my_vector;
std::vector<std::vector<uint64_t>> my_big_vector;

...

// For 1D
my_generator.generate(my_vector);
// For 2D
my_generator.generate_2D(my_big_vector);
```

The vectors will then be filled. In the next release a proper implementation to get rid of the need for separate
1D and 2D functions, including 32-bit and 64-bit ints, will be added.

For numbers in the range [0,1) (exclusive of 1 itself)

```
my_generator.generate(my_double_vector)
```
where my_double_vector is a vector (or a vector of vectors, or an array or arrays!) of doubles.

For a number within a range [lower:upper)

```
unsigned int lower = 5;
unsigned int upper = 500;
generate_range(my_vector, lower, upper);
```

### Example

```
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

	// Create a Threaded_rands object
	Threaded_rands<result_type, state_type> my_generator(selection, n_threads);

	// Fill the vector
	my_generator.generate(vector_storage);
}
```

### Limitations

There are many limitations currently, including the use of only OpenMP for
very simple multi-threading. In the next release threading will be properly implemented.

The JSF generator does not have a jump ahead function for non-overlapping streams of parallel
numbers. Due to this limitation that generator should not currently be used for large numbers of parallel
threads.


### Acknowledgements

Prof. M. O'Neil providing the PCG PRNG and C++ implementation
of the JSF PRNG, both available from http://www.pcg-random.org/

David Blackman and Sebastiano Vigna for their xo* family of PRNGs
available at http://xoshiro.di.unimi.it/

