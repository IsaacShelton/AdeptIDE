
#ifndef PSEUDO_TERMINAL_H
#define PSEUDO_TERMINAL_H

#include <mutex>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include "INTERFACE/Settings.h"

class PseudoTerminal {
public:
    static PseudoTerminal* create(Settings *settings);

    PseudoTerminal();
    virtual ~PseudoTerminal();
    virtual void feedInput(const std::string food) = 0;
    virtual void feedInput(unsigned int codepoint) = 0;
    virtual std::string readOutput() = 0;
};

class WindowsCMD : public PseudoTerminal {
    std::mutex mutex;
    std::string shell;
    std::vector<std::string> arguments;
    std::string input, output;
    std::atomic_bool failed;
    std::atomic_bool shouldStop;
    std::thread thread;

public:
    WindowsCMD(Settings* settings);
    WindowsCMD(const std::string& shell, const std::vector<std::string> arguments);
    ~WindowsCMD();

    void feedInput(const std::string food);
    void feedInput(unsigned int codepoint);
    std::string readOutput();
};

class UnixPTY : public PseudoTerminal {
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
    UnixPTY(Settings* settings);
    UnixPTY(const std::string& shell, const std::vector<std::string> arguments, const std::string& term);
    ~UnixPTY();

    void feedInput(const std::string food);
    void feedInput(unsigned int codepoint);
    std::string readOutput();
};

#endif
