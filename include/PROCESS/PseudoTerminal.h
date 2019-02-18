
#ifndef PSEUDO_TERMINAL_H
#define PSEUDO_TERMINAL_H

#include <mutex>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include "INTERFACE/Settings.h"

class PseudoTerminal {
private:
    std::mutex mutex;
    std::string shell;
    std::vector<std::string> arguments;
    int fdMaster, fdSlave;
    std::string input, output;
    std::atomic_bool failed;
    std::atomic_bool shouldStop;
    std::thread thread;
    std::string term;

public:
    PseudoTerminal(Settings* settings);
    PseudoTerminal(const std::string& shell, const std::vector<std::string> arguments, const std::string& term);
    ~PseudoTerminal();

    void feedInput(const std::string food);
    void feedInput(unsigned int codepoint);
    std::string readOutput();
};

#endif
