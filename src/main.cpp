#include <stdio.h>
#include "game/Game.hpp"

void ParseArg(const int argc, const char* argv[]);

int main(const int argc, const char* argv[]) { 
    // ParseArg(argc, argv); 

    Game akinator = {};
    RunGame(&akinator);

    return 0;
}

void ParseArg(const int argc, const char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        printf("%s ", argv[i]);
    }

    printf("\n\n");
}
