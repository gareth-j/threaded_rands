// System seeder - get random numbers for seeding PRNGs from your operating system
// Copyright (C) 2018 Gareth Jones

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
#ifndef SYSTEM_SEED_H
#define SYSTEM_SEED_H

#include <iostream>
#include <fstream>
#include <iterator>
#include <cerrno>
#include <cstring>


// Some of the code in this file was taken/modified from answers
// to the Stackoverflow question
// https://stackoverflow.com/q/45069219/10354589

// Some ugly preprocessor directives to ensure the correct includes
#if defined(__linux__) || defined(linux) || defined(__linux)
#include <linux/version.h>
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0) && __GLIBC__ > 2 || __GLIBC_MINOR__ > 24
       #define HAVE_GETRANDOM
       #include <sys/random.h>
	#else
		#define USE_DEV_RANDOM
  #endif

#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__) || defined(__MACH__)
#include <cstdlib>
#define HAVE_BSD_TYPE

#elif defined(_WIN64) || defined(_WIN32)
#define HAVE_WINDOWS
#include <windows.h>
#include <Wincrypt.h>
#endif

class system_seed
{
public:
	system_seed(){} // Do nothing here at the moment

	// Linux kernel version 3.17 introduced getrandom()
	// A version of glibc >= 2.25 is also required
	#if defined(HAVE_GETRANDOM)

	template <typename Iter>
	void generate(Iter begin, Iter end)
	{	
		typename std::iterator_traits<Iter>::value_type rand = *begin;

		size_t rand_size = sizeof(rand);

		// Flag to control the source of entropy and blocking 
		unsigned int flag = 0;

		for(Iter iter = begin; iter != end; ++iter) 
		{
			ssize_t result = getrandom(&rand, rand_size, flag);
			
			if (result == -1)
				std::cerr << "Error - unable to get random number\n" << std::strerror(errno) << "\n";

			// As long as we haven't got an error
			// and the fn has returned bytes
			if(result > 0)
				*iter = rand;		
		}	
	}

	// For older versions of Linux we fall back to /dev/urandom
	#elif defined(USE_DEV_RANDOM)

	template <typename Iter>
	void generate(Iter begin, Iter end)
	{	
		typename std::iterator_traits<Iter>::value_type rand = *begin;

		size_t rand_size = sizeof(rand);

	    std::ifstream rand_stream("/dev/urandom", std::ios::in | std::ios::binary); 
	    if(rand_stream)
	    {
			for(Iter iter = begin; iter != end; ++iter)
			{
		    	rand_stream.read(reinterpret_cast<char*>(&rand), rand_size);
		    	if(rand_stream)
			        *iter = rand;
			    else
			        std::cerr << "Failed to read /dev/urandom" << std::endl;			
			}	    	
	    }
	    else
	    	std::cerr << "Unable to open /dev/urandom.\n";    
	}

	// The BSDs and OS X / MacOS should come with arc4random_buf
	// which allows us to get a cryptographic pseudo-random number
	// On newer BSDs it will use the ChaCha20 cipher
	// On older BSDs it will use ARC4 cipher
	#elif defined(HAVE_BSD_TYPE)

	template <typename Iter>
	void generate(Iter begin, Iter end)
	{
		typename std::iterator_traits<Iter>::value_type rand = *begin;
		
		size_t rand_size = sizeof(rand);

		for(Iter iter = begin; iter != end; ++iter)
		{
			arc4random_buf(&rand, rand_size);

			*iter = rand;
		}
	}

	#elif defined(HAVE_WINDOWS)

	template <typename Iter>
	void generate(Iter begin, Iter end)
	{
		typename std::iterator_traits<Iter>::value_type rand = *begin;
		
		size_t rand_size = sizeof(rand);

		// Create a cryptographic service provider object
		HCRYPTPROV ctx;

		if (!CryptAcquireContext(ctx, nullptr, nullptr, PROV_RSA_FULL, 0)) 
	        std::cerr << "Unable to initialize Windows random number provider.\n";

	    BYTE* buffer = reinterpret_cast<BYTE*>(&rand);

	    for(Iter iter = begin; iter != end; ++iter)
	    {
			bool success = CryptGenRandom(ctx, rand_size, buffer);

			if(!success)
	        	std::cerr << "Unable to generate random bytes.\n";
	        else
	        	*iter = rand;
	    }

	    // Try and release the object
	    if (!CryptReleaseContext(ctx, 0)) 
	        std::cerr << "Unable to release random number provider.\n";

	}
	#endif
};

#endif // SYSTEM_SEED_H