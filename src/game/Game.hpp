#ifndef SRC_GAME_GAME_HPP_
#define SRC_GAME_GAME_HPP_

#include "../tree/BinaryTree.hpp"

struct Game {
    enum State {
        ConsoleMode,
        StartMenu,
    } state_;

    BinTree tree_ = {};
    BinTree::Node *active_node_ = nullptr;
};

void RunGame(Game *game);

#endif // SRC_GAME_GAME_HPP_