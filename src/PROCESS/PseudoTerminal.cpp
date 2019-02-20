
//#define _XOPEN_SOURCE 600
//#define __USE_BSD
//#define __BSD_SOURCE
//#define __DEFAULT_SOURCE
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#ifndef _WIN32
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#endif

#include <string.h>

#include "PROCESS/PseudoTerminal.h"

PseudoTerminal::PseudoTerminal(Settings* settings) : PseudoTerminal(
        settings->terminal_shell,
        settings->terminal_shell_arguments,
        settings->terminal_environment_term
){}

PseudoTerminal::PseudoTerminal(const std::string& shell, const std::vector<std::string> arguments, const std::string& term){
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

        // Create the child process
        if(fork()){
            // FATHER

            fd_set fd_in;

            // Close the slave side of the PTY
            close(this->fdSlave);

            char input[256 + 1];

            while(!this->shouldStop.load()){
                this->mutex.lock();
                // Wait for data from standard input and master side of PTY
                FD_ZERO(&fd_in);
                //FD_SET(STDIN_FILENO, &fd_in);
                FD_SET(this->fdMaster, &fd_in);

                struct timeval timeout = (struct timeval){0, 0};

                switch(select(this->fdMaster + 1, &fd_in, NULL, NULL, &timeout)){
                case -1:
                    fprintf(stderr, "Error %d on select()\n", errno);
                    exit(1);
                default:
                    // If data on standard input
                    //if(FD_ISSET(STDIN_FILENO, &fd_in)){
                    //    res = read(STDIN_FILENO, input, sizeof(input) - 1);
                    //    if(res > 0){
                    //        // Send data on the master side of PTY
                    //        write(this->fdMaster, input, res);
                    //        input[res] = '\0';
                    //        this->output += input;
                    //    }
                    //    else if(res < 0){
                    //        fprintf(stderr, "Error %d on read standard input\n", errno);
                    //        exit(1);
                    //    }
                    //}

                    // If data on master side of PTY
                    if (FD_ISSET(this->fdMaster, &fd_in)){
                        res = read(this->fdMaster, input, sizeof(input) - 1);
                        if(res > 0){
                            // Send data on standard output
                            input[res] = '\0';
                            this->output += input;
                            //write(STDOUT_FILENO, input, res);
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
                    //this->output += this->input.substr(0, chunk_length);
                    this->input.erase(0, chunk_length);
                }

                this->mutex.unlock();
            }

            this->mutex.lock();
            close(this->fdMaster);
            this->mutex.unlock();
        } else {
            // CHILD

            struct termios slave_orig_term_settings; // Saved terminal settings
            struct termios new_term_settings;        // Current terminal settings

            // Close the master side of the PTY
            close(this->fdMaster);

            // Save the defaults parameters of the slave side of the PTY
            res = tcgetattr(this->fdSlave, &slave_orig_term_settings);

            // Set RAW mode on slave side of PTY
            new_term_settings = slave_orig_term_settings;
            cfmakeraw(&new_term_settings);
            tcsetattr(this->fdSlave, TCSANOW, &new_term_settings);

            // The slave side of the PTY becomes the standard input and outputs of the child process
            close(0); // Close standard input (current terminal)
            close(1); // Close standard output (current terminal)
            close(2); // Close standard error (current terminal)

            dup(this->fdSlave); // PTY becomes standard input (0)
            dup(this->fdSlave); // PTY becomes standard output (1)
            dup(this->fdSlave); // PTY becomes standard error (2)

            // Now the original file descriptor is useless
            close(this->fdSlave);

            // Make the current process a new session leader
            setsid();

            // As the child is a session leader, set the controlling terminal to be the slave side of the PTY
            // (Mandatory for programs like the shell to make them manage correctly their outputs)
            ioctl(0, TIOCSCTTY, 1);

            // Execution of the program
            {
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
            }

            // if Error...
            this->failed.store(true);
            return;
        }
    });
    #else
    // _WIN32
    this->thread = std::thread([this]() -> void {
        // Pseudo terminal unimplemented for windows
    });
    #endif
}

PseudoTerminal::~PseudoTerminal(){
    // Obtain control
    this->shouldStop.store(true);
    this->thread.join();
    // Clean up
}

void PseudoTerminal::feedInput(const std::string food){
    this->mutex.lock();
    this->input += food;
    this->mutex.unlock();
}

void PseudoTerminal::feedInput(unsigned int codepoint){
    this->mutex.lock();
    this->input += codepoint;
    this->mutex.unlock();
}

std::string PseudoTerminal::readOutput(){
    this->mutex.lock();
    std::string output = this->output;
    this->output = "";
    this->mutex.unlock();
    return output;
}
