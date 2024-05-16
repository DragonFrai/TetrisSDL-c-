#include <iostream>
#include <chrono>
#include <thread>
#include <SDL2/SDL.h>
#include "app.cpp"

using namespace std::chrono_literals;

//Screen dimension constants

int main( int argc, char* args[] )
{
    std::srand(time(NULL));

    auto app = Tetris_initApplication();

    bool doLoop = true;
    while (doLoop) {
        std::this_thread::sleep_for(20ms);
        doLoop = app.tick(0.02);
    }

    Tetris_closeApplication(&app);

    return 0;
}