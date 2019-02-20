
#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <mutex>
#include <atomic>
#include <thread>
#include <string>

// Responsible for background operations

struct BackgroundInput {
    std::mutex mutex;
    std::atomic_bool should_close;
    std::atomic_bool updated;
    std::string text;
    std::string filename;
};

struct BackgroundOutput {
    std::mutex mutex;
    std::atomic_bool updated;
    size_t newlines = 0;
};

extern BackgroundInput global_background_input;
extern BackgroundOutput global_background_output;
extern std::thread *global_background_thread;

void background();

void printParsed(const std::string& filename, const std::string& text);

#endif // BACKGROUND_H
