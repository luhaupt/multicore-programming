#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "ALock.hpp"
#include "ALog.hpp"
#include "BackoffLock.hpp"
#include "Lock.hpp"
#include "MCSlock.hpp"
#include "TASlock.hpp"
#include "TTASlock.hpp"
#include "Ticker.hpp"

constexpr std::size_t INCREMENTS = 100000;

struct Config {
    std::size_t jobs = 1;
    std::size_t samples = 1;
    std::string lock;

    std::size_t backoff_min = 0;
    std::size_t backoff_max = 0;
};

/**
 * Helper function to parse all CLI arguments and return a config struct.
 *
 * @return A struct containing the configuration setup of the application.
 */
Config parse_arguments(int argc, char *argv[]) {
    Config config;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--jobs") {
            if (i + 1 >= argc)
                throw std::runtime_error("--jobs requires a value");

            config.jobs = std::stoull(argv[++i]);

            if (config.jobs == 0)
                throw std::runtime_error("--jobs must be larger than 0");
        } else if (arg == "--samples") {
            if (i + 1 >= argc)
                throw std::runtime_error("--samples requires a value");

            config.samples = std::stoull(argv[++i]);

            if (config.samples == 0)
                throw std::runtime_error("--samples must be larger than 0");
        } else if (arg == "--lock") {
            if (i + 1 >= argc)
                throw std::runtime_error("--lock requires a value");

            config.lock = argv[++i];
        } else if (arg == "--backoff-min") {
            if (i + 1 >= argc)
                throw std::runtime_error("--backoff-min requires a value");

            config.backoff_min = std::stoull(argv[++i]);
        } else if (arg == "--backoff-max") {
            if (i + 1 >= argc)
                throw std::runtime_error("--backoff-max requires a value");

            config.backoff_max = std::stoull(argv[++i]);
        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    if (config.lock.empty()) {
        throw std::runtime_error("Missing required argument: --lock");
    }

    if (config.backoff_min > config.backoff_max) {
        throw std::runtime_error(
            "backoff-min cannot be larger than backoff-max");
    }

    return config;
}

/**
 * Create a lock for a valid type identifier.
 *
 * @return A unique pointer with the desired Lock class.
 */
std::unique_ptr<Lock> create_lock(const Config &config) {
    if (config.lock == "tas") {
        return std::make_unique<TASlock>();
    }

    if (config.lock == "ttas") {
        return std::make_unique<TTASlock>();
    }

    if (config.lock == "aq") {
        return std::make_unique<ALock>(config.jobs);
    }

    if (config.lock == "alog") {
        return std::make_unique<ALog>(config.jobs);
    }

    if (config.lock == "mcs") {
        return std::make_unique<MCSLock>();
    }

    if (config.lock == "backoff") {
        return std::make_unique<BackoffLock>(config.backoff_min,
                                             config.backoff_max);
    }

    throw std::runtime_error("Unknown lock: " + config.lock);
}

/**
 * Create a ticker for a valid type identifier.
 *
 * @return A unique pointer with the desired Ticker class.
 */
std::unique_ptr<Ticker> create_ticker(const std::string &type, Lock *lock) {
    if (type == "atomic") {
        return std::make_unique<STLAtomicTicker>();
    } else if (lock != nullptr) {
        return std::make_unique<LockedTicker>(*lock);
    }

    throw std::runtime_error("Unknown ticker: " + type);
}

/**
 * Run a sample with the given config and provided lock.
 * While execution it measures the runtime and prints it to std::cout
 * afterwards.
 */
void run_sample(Config &config, Ticker &ticker) {
    // Allocate container before measurement
    std::vector<std::jthread> threads;
    threads.reserve(config.jobs);

    // Measure runtime
    auto start = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < config.jobs; ++i) {
        threads.emplace_back([&ticker]() {
            for (std::size_t j = 0; j < INCREMENTS; ++j) {
                ticker++;
            }
        });
    }

    // Wait for all threads
    for (auto &thread : threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto runtime =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            .count();

    // Output result
    std::string lock_name = config.lock;

    if (config.lock == "backoff") {
        lock_name = "backoff-" + std::to_string(config.backoff_min) + "-" +
                    std::to_string(config.backoff_max);
    }
    std::cout << config.jobs << ',' << lock_name << ',' << runtime << '\n';
}

int main(int argc, char *argv[]) {
    Config config;
    std::unique_ptr<Lock> lock = nullptr;

    try {
        config = parse_arguments(argc, argv);

        for (std::size_t sample = 0; sample < config.samples; ++sample) {
            if (config.lock != "atomic") {
                lock = create_lock(config);
            }

            auto ticker = create_ticker(config.lock, lock.get());

            // Run a sample once
            run_sample(config, *ticker);
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
