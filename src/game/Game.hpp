#ifndef SRC_GAME_GAME_HPP_
#define SRC_GAME_GAME_HPP_

#include "../tree/BinaryTree.hpp"
#include "../engine/Engine.hpp"

struct Game {
    enum State {
        ConsoleMode,
        StartMenu,
    } state_;

    BinTree tree_ = {};
    BinTree::Node *active_node_ = nullptr;

    Engine engine_ = {};
};

void RunGame(Game *game);

#endif // SRC_GAME_GAME_HPP_