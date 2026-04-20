#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>
#include <syncstream>

using usize = std::size_t;
using u64 = std::uint64_t;

class MutexCounter {
private:
    int value{0};
    std::mutex mtx;

public:
    /**
     * Returns the current counter number and increments it.
     */
    int get_and_increment() {
    	std::lock_guard<std::mutex> lock(mtx);
        return value++;
    }
};

class AtomicCounter {
private:
    std::atomic<int> value{0};

public:
    /**
     * Returns the current counter number and increments it.
     */
    int get_and_increment() {
        return value++;
    }
};

const usize N = 100'000'000;

/**
 *  Helper function to check whether the CLI argument "jobs" is from type int.
 */
bool parse_int(std::string_view sv, usize& out) {
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);

    return ec == std::errc() && ptr == sv.data() + sv.size();
}

// (a * b) % mod without overflow
usize mod_mul(const usize& a, const usize& b, const usize& mod) {
	__uint128_t res = (__uint128_t)a * b;
	return (usize)(res % mod);
}

// (base^exp) % mod
usize mod_pow(usize base, u64 exp, const usize& mod) {
	usize result = 1;
	while (exp > 0) {
		if (exp & 1) {
			result = mod_mul(result, base, mod);
		}
		base = mod_mul(base, base, mod);
		exp >>= 1;
	}
	return result;
}

/**
 * Checks whether a number is a prime with the Miller-Rabin test.
 */
bool miller_rabin(const usize& n) {
	if (n < 2) return false;

	// Check small primes fast
	for (usize p : {2, 3, 5, 7}) {
		if (n % p == 0) return n == p;
	}

	u64 d = n - 1;
	int s = 0;
	while((d & 1) == 0) {
		d >>= 1;
		++s;
	}

	auto check = [&](usize a) {
		if (a % n == 0) return true;

		usize x = mod_pow(a, d, n);
		if (x == 1 || x == n - 1) return true;

		for (int r = 1; r < s; ++r) {
			x = mod_mul(x, x, n);
			if (x == n - 1) return true;
		}
		return false;
	};

	// Check only bases necessary for our N
	for (usize a : {2, 7, 61}) {
		if (!check(a)) return false;
	}

	return true;
}

/**
 * Checks whether a number is a prime.
 */
void print_prime(const usize& n, std::atomic<usize>& cnt) {
	if(!miller_rabin(n)) return;

	cnt.fetch_add(1, std::memory_order_relaxed);
	// std::osyncstream(std::cout) << n << "\n";
}

/**
 * Calculates prime numbers by equally sized chunks.
 */
void process_chunks(const usize& number_of_threads, std::atomic<usize>& cnt) {
	// Calculate chunk size to process for each thread
    std::vector<std::jthread> threads;
    usize chunk_size = (N + number_of_threads - 1) / number_of_threads;

    for (usize thread_id = 0; thread_id < number_of_threads; ++thread_id) {
        usize start = thread_id * chunk_size;
        usize end = std::min(start + chunk_size, N);

        if (start >= N) break;

        threads.emplace_back([start, end, &cnt] {
            for (usize i = start; i < end; ++i) {
            	print_prime(i, cnt);
            }
        });
    }
}

/**
 * Calculates prime numbers using a shared counter with a mutex.
 */
void process_shared_mutex(const usize& number_of_threads, std::atomic<usize>& cnt) {
	std::vector<std::jthread> threads;
    MutexCounter counter;

    for (size_t thread_id = 0; thread_id < number_of_threads; ++thread_id) {
        threads.emplace_back([&counter, &cnt] {
            while(true) {
                usize current_number = counter.get_and_increment();

                if(current_number >= N) break;

                print_prime(current_number, cnt);
            }
        });
    }
}

/**
 * Calculates prime numbers using a shared counter with atomic.
 */
void process_shared_atomic(const usize& number_of_threads, std::atomic<usize>& cnt) {
	std::vector<std::jthread> threads;
    AtomicCounter counter;

    for (size_t thread_id = 0; thread_id < number_of_threads; ++thread_id) {
        threads.emplace_back([&counter, &cnt] {
            while(true) {
                usize current_number = counter.get_and_increment();

                if(current_number >= N) break;

                print_prime(current_number, cnt);
            }
        });
    }
}

int main(int argc, char *argv[]) {
    if (argc > 3) {
        std::cout << "Usage: ./my-primes [--jobs <number of threads>, Default: 1]\n";
        return EXIT_FAILURE;
    }

    usize number_of_threads = 1;
    bool argument_provided = false;
    std::atomic<usize> cnt = 0;

    if (argc > 1) {
        argument_provided = std::strcmp(argv[1], "--jobs") == 0;

        if (!argument_provided) {
            std::cout << "Warning: Unknown option '" << argv[1] << "'. Defaulting to 1 thread...\n";
        }
    }

    if (argc == 2 && argument_provided) {
        std::cout << "Warning: No number of threads provided. Defaulting to 1 thread...\n";
    }

    if (argc == 3 && argument_provided) {
        if (!parse_int(argv[2], number_of_threads)) {
            std::cerr << "Error: Invalid number format\n";
            return EXIT_FAILURE;
        }

        if (number_of_threads < 1 || number_of_threads > 10) {
            std::cerr << "Error: The number of threads must be between 1 and 10\n";
            return EXIT_FAILURE;
        }
    }

    // process_chunks(number_of_threads, cnt);
    // process_shared_mutex(number_of_threads, cnt);
    process_shared_atomic(number_of_threads, cnt);
}
