// #include "include/pcg/pcg_random.hpp"
// #include "include/pcg/randutils.hpp"
#include "include/benchmark.hpp"
#include <iostream>
#include <vector>
#include <random>

int main()
{
	// pcg64wrap gen;

	benchmark bench;

	bench.benchmark_generators();
}