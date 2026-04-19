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

const size_t N = 100'000'000;

/**
 *  Helper function to check whether the CLI argument "jobs" is from type int.
 */
bool parse_int(std::string_view sv, size_t& out) {
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);

    return ec == std::errc() && ptr == sv.data() + sv.size();
}

/**
 * Checks whether a number is a prime.
 * The calculation only considers dividers below the numbers root and increases a potential divider by 2.
 *
 * Returns true if the number is a prime, false otherwise.
 */
bool is_prime(const size_t& number) {
    if (number <= 1) return false;
    if (number == 2) return true;
    if (number % 2 == 0) return false;

    for (size_t i = 3; i * i <= number; i += 2) {
        if (number % i == 0) return false;
    }

    return true;
}

int main(int argc, char *argv[]) {
    if (argc > 3) {
        std::cout << "Usage: ./my-primes [--jobs <number of threads>, Default: 1]\n";
        return EXIT_FAILURE;
    }

    size_t number_of_threads = 1;
    bool argument_provided = false;

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

    std::cout << "Starting with " << number_of_threads << (number_of_threads == 1 ? " thread" : " threads") << "...\n";

    // Calculate chunk size to process for each thread
    std::vector<std::jthread> threads;
    size_t chunk_size = (N + number_of_threads - 1) / number_of_threads;
    std::cout << "Chunk size: " << chunk_size << "\n";

    std::mutex cout_mutex;

    for (size_t thread_id = 0; thread_id < number_of_threads; ++thread_id) {
        size_t start = thread_id * chunk_size;
        size_t end = std::min(start + chunk_size, N);

        if (start >= N) break;

        threads.emplace_back([start, end, thread_id, &cout_mutex] {
            for (size_t i = start; i < end; ++i) {
                if (is_prime(i)) {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << i << "\n";
                }
            }
        });
    }
}
