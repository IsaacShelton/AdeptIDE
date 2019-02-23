
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#ifndef _WIN32
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <string.h>

#include "PROCESS/PseudoTerminal.h"

PseudoTerminal::PseudoTerminal(){}
PseudoTerminal::~PseudoTerminal(){}

UnixPTY::UnixPTY(Settings* settings) : UnixPTY(
        settings->terminal_shell,
        settings->terminal_shell_arguments,
        settings->terminal_environment_term
){}

UnixPTY::UnixPTY(const std::string& shell, const std::vector<std::string> arguments, const std::string& term){
    this->shell = shell;
    this->arguments = arguments;
    this->fdMaster = -1;
    this->fdSlave = -1;
    this->input = "";
    this->output = "";
    this->failed.store(false);
    this->shouldStop.store(false);
    this->term = term;

    #ifndef _WIN32
    this->thread = std::thread([this]() -> void {
        this->mutex.lock();
        int res;

        this->fdMaster = posix_openpt(O_RDWR);
        if(this->fdMaster < 0){
            this->failed.store(true);
            return;
        }

        if(grantpt(this->fdMaster) != 0 || unlockpt(this->fdMaster) != 0){
            this->failed.store(true);
            return;
        }

        this->fdSlave = open(ptsname(this->fdMaster), O_RDWR);
        this->mutex.unlock();

        if(fork()){
            // PARENT
            close(this->fdSlave);

            fd_set fd_in;
            char input[256 + 1];

            while(!this->shouldStop.load()){
                this->mutex.lock();

                FD_ZERO(&fd_in);
                FD_SET(this->fdMaster, &fd_in);

                struct timeval timeout = (struct timeval){0, 0};

                switch(select(this->fdMaster + 1, &fd_in, NULL, NULL, &timeout)){
                case -1:
                    fprintf(stderr, "Error %d on select()\n", errno);
                    exit(1);
                default:
                    if (FD_ISSET(this->fdMaster, &fd_in)){
                        res = read(this->fdMaster, input, sizeof(input) - 1);
                        if(res > 0){
                            input[res] = '\0';
                            this->output += input;
                        } else if(res < 0){
                            fprintf(stderr, "Error %d on read master PTY\n", errno);
                            exit(1);
                        }
                    }
                }

                if(this->input != ""){
                    size_t chunk_length = this->input.length() > 256 ? 256 : this->input.length();
                    memcpy(input, &this->input[0], chunk_length);
                    input[chunk_length] = '\0';
                    write(this->fdMaster, input, chunk_length);
                    this->input.erase(0, chunk_length);
                }

                this->mutex.unlock();
            }

            this->mutex.lock();
            close(this->fdMaster);
            this->mutex.unlock();
        } else {
            // CHILD

            struct termios slave_orig_term_settings;
            struct termios new_term_settings;

            close(this->fdMaster);
            res = tcgetattr(this->fdSlave, &slave_orig_term_settings);

            new_term_settings = slave_orig_term_settings;
            cfmakeraw(&new_term_settings);
            tcsetattr(this->fdSlave, TCSANOW, &new_term_settings);

            close(0);
            close(1);
            close(2);

            dup(this->fdSlave);
            dup(this->fdSlave);
            dup(this->fdSlave);
            close(this->fdSlave);
            
            setsid();
            ioctl(0, TIOCSCTTY, 1);

            char *child_argv[2 + this->arguments.size()];

            // Build the command line
            child_argv[0] = strdup(this->shell.c_str());
            for(size_t i = 0; i != this->arguments.size(); i++){
                child_argv[i + 1] = strdup(this->arguments[i].c_str());
            }
            child_argv[1 + this->arguments.size()] = NULL;

            char term_env[5 + this->term.length() + 1];
            memcpy(term_env, "TERM=", 5);
            memcpy(&term_env[5], this->term.c_str(), this->term.length() + 1);
            putenv(term_env);
            res = execvp(child_argv[0], child_argv);

            this->failed.store(true);
            return;
        }
    });
    #else
    // _WIN32
    this->thread = std::thread([this]() -> void {
        // Unix PTY is not available on Windows
    });
    #endif
}

UnixPTY::~UnixPTY(){
    // Obtain control
    this->shouldStop.store(true);
    this->thread.join();
    // Clean up
}

void UnixPTY::feedInput(const std::string food){
    this->mutex.lock();
    this->input += food;
    this->mutex.unlock();
}

void UnixPTY::feedInput(unsigned int codepoint){
    this->mutex.lock();
    this->input += codepoint;
    this->mutex.unlock();
}

std::string UnixPTY::readOutput(){
    this->mutex.lock();
    std::string output = this->output;
    this->output = "";
    this->mutex.unlock();
    return output;
}
