#include <assert.h>
#include <string.h>
#include "Game.hpp"

#include <stdio.h>
#define Error(ERROR_TEXT) fprintf(stderr, "GameError: " ERROR_TEXT "\n");

// Game::private:
static void Init(Game *game) {
    assert(game);

    //game->state_ = Game::StartMenu;
    game->state_ = Game::ConsoleMode;

    BinTreeInsert(&game->tree_.head_, strdup("Y or N"));
    BinTreeInsert(&game->tree_.head_->L, strdup("YES"));
    BinTreeInsert(&game->tree_.head_->R, strdup("NO"));
}

static bool ActiveNodeIsTerminal(Game *game) {
    assert(game);
    assert(game->active_node_);

    return game->active_node_->L == nullptr && game->active_node_->R == nullptr;
}

static const char* GetQuestion(Game *game) {\
    assert(game);
    assert(game->active_node_);

    return (const char*)game->active_node_->data;
}

static const char* GetDescription(Game *game) {
    assert(game);
    assert(game->active_node_);

    return (const char*)game->active_node_->data;
}

void static inline BufferFlush() {
    while (getc(stdin) != '\n')
        ;
}

static void ConsoleCicle(Game *game) {
    game->active_node_ = game->tree_.head_;

    while (!ActiveNodeIsTerminal(game)) {
        printf("It is %s? (Y/N)\n", GetQuestion(game));
        
        char c = 0;
        if ((c = getc(stdin)) == 'Y') {
            game->active_node_ = game->active_node_->L;
        } else if (c == 'N'){
            game->active_node_ = game->active_node_->R;
        } else {
            puts("Uncorrect answer :(\n");
        }

        BufferFlush();
    }

    printf("It is: %s!\n", GetDescription(game));
}

static void MainLoop(Game *game) {
    assert(game);

    while (true) { // TODO:
        switch (game->state_) {
        case Game::ConsoleMode:
            ConsoleCicle(game);

            puts("You want to play again? (Y/N)\n");
            if (getc(stdin) == 'N') return;
            BufferFlush();
            break;

        case Game::StartMenu:

            //break;
        
        default:
            Error("Find undefined game state");
            return; // TODO:
            // abort()
            break;
        }
    }
}

// Game::public:
void RunGame(Game *game) {
    assert(game);

    Init(game);
    MainLoop(game);
}
