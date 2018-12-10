
#include <algorithm>
#include "PROCESS/background.h"

BackgroundInput global_background_input;
BackgroundOutput global_background_output;
std::thread *global_background_thread = NULL;

void background(){
    // Background thread

    while(!global_background_input.should_close){
        while(!global_background_input.updated.load() && !global_background_input.should_close.load()){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if(global_background_input.should_close.load()) break;

        global_background_input.mutex.lock();
        global_background_input.updated.store(false);
        std::string text = global_background_input.text;
        std::string filename = global_background_input.filename;
        global_background_input.mutex.unlock();

        global_background_output.mutex.lock();
        global_background_output.newlines = std::count(text.begin(), text.end(), '\n');
        global_background_output.updated.store(true);
        global_background_output.mutex.unlock();
    }
}
