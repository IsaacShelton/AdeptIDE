
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#endif

#include <string.h>
#include "PROCESS/PseudoTerminal.h"

PseudoTerminal* PseudoTerminal::create(Settings *settings){
    #if _WIN32
    return new WindowsCMD(settings);
    #else
    return new UnixPTY(settings);
    #endif
    return NULL;
}

PseudoTerminal::PseudoTerminal(){}
PseudoTerminal::~PseudoTerminal(){}

WindowsCMD::WindowsCMD(Settings* settings) : WindowsCMD(
        settings->terminal_shell,
        settings->terminal_shell_arguments
){}

WindowsCMD::WindowsCMD(const std::string& shell, const std::vector<std::string> arguments){
    this->shell = shell;
    this->arguments = arguments;
    this->input = "";
    this->output = "";
    this->failed.store(false);
    this->shouldStop.store(false);
    
    #ifdef _WIN32
    this->thread = std::thread([this]() -> void {
        char buf[512];

        STARTUPINFOA si;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;
        PROCESS_INFORMATION pi;
        HANDLE newstdin, newstdout, read_stdout, write_stdin;

        OSVERSIONINFO osv;
        osv.dwOSVersionInfoSize = sizeof(osv);
        GetVersionEx(&osv);

        if(osv.dwPlatformId == VER_PLATFORM_WIN32_NT){
            InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
            SetSecurityDescriptorDacl(&sd, true, NULL, false);
            sa.lpSecurityDescriptor = &sd;
        } else {
            sa.lpSecurityDescriptor = NULL;
        }

        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = true;

        if(!CreatePipe(&newstdin, &write_stdin, &sa, 0)){
            this->failed.store(true);
            return;
        }

        if(!CreatePipe(&read_stdout, &newstdout, &sa, 0)){
            this->failed.store(true);
            CloseHandle(newstdin);
            CloseHandle(write_stdin);
            return;
        }

        GetStartupInfoA(&si);
        
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = newstdout;
        si.hStdError = newstdout;
        si.hStdInput = newstdin;

        if(!CreateProcessA(this->shell.c_str(), NULL, NULL, NULL, true, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)){
            this->failed.store(true);
            CloseHandle(newstdin);
            CloseHandle(newstdout);
            CloseHandle(read_stdout);
            CloseHandle(write_stdin);
            return;
        }

        unsigned long exitcode, bytes_read, available;
        memset(buf, 0, sizeof(buf));

        while(!this->shouldStop.load()){
            GetExitCodeProcess(pi.hProcess, &exitcode);
            if(exitcode != STILL_ACTIVE) break;

            PeekNamedPipe(read_stdout, buf, sizeof(buf) - 1, &bytes_read, &available, NULL);

            this->mutex.lock();
            if(bytes_read != 0){
                memset(buf, 0, sizeof(buf));
                if(available > sizeof(buf) - 1){
                    while(bytes_read >= sizeof(buf) - 1){
                        ReadFile(read_stdout, buf, sizeof(buf) - 1, &bytes_read, NULL);
                        this->output += buf;
                        memset(buf, 0, sizeof(buf));
                    }
                } else {
                    ReadFile(read_stdout, buf, sizeof(buf) - 1, &bytes_read, NULL);
                    this->output += buf;
                }
            }

            if(this->input != ""){
                size_t chunk_length = this->input.length() > sizeof(buf) - 1 ? sizeof(buf) - 1 : this->input.length();
                memcpy(buf, &this->input[0], sizeof(buf) - 1);
                input[sizeof(buf)] = '\0';
                WriteFile(write_stdin, buf, chunk_length, &bytes_read, NULL);
                this->input.erase(0, chunk_length);
            }

            this->mutex.unlock();
            Sleep(10);
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseHandle(newstdin);
        CloseHandle(newstdout);
        CloseHandle(read_stdout);
        CloseHandle(write_stdin);
    });
    #else
    // not _WIN32
    this->thread = std::thread([/*this*/]() -> void {
        // Windows CMD is not available on non-Windows platforms
    });
    #endif
}

WindowsCMD::~WindowsCMD(){
    // Obtain control
    this->shouldStop.store(true);
    this->thread.join();
    // Clean up
}

void WindowsCMD::feedInput(const std::string food){
    this->mutex.lock();
    this->input += food;
    this->mutex.unlock();
}

void WindowsCMD::feedInput(unsigned int codepoint){
    this->mutex.lock();
    this->input += codepoint;
    this->mutex.unlock();
}

std::string WindowsCMD::readOutput(){
    this->mutex.lock();
    std::string output = this->output;
    this->output = "";
    this->mutex.unlock();
    return output;
}

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
                
                // EXPERIMENTAL: Sleep for 10ms
                /*
                struct timespec ts;
                ts.tv_sec = 0;
                ts.tv_nsec = 10000000;
                nanosleep(&ts, NULL);
                */
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
