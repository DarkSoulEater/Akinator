#include <assert.h>
#include <string.h>
#include "Buffer.hpp"
#include "HashTable.hpp"
#include "Game.hpp"

#define T BinTree::Node*
#include "LongArray.hpp"
#include "Stack.hpp"

#include <stdio.h>
#define Error(ERROR_TEXT) fprintf(stderr, "GameError: " ERROR_TEXT "\n");

const char* kSaveFileName = "bd\\tree_save.txt";  

// Game::private:
static void ReadTreeFromFile(Game *game, const char* file_name) {
    assert(game);
    assert(file_name);

    Buffer *buffer = CreateBufferFromFile(file_name);

    char* data = (char*) BufferGetData(buffer);
    size_t buffer_size = BufferGetSize(buffer);

    for (size_t i = 0; i < buffer_size; ++i) {
        if (data[i] == '\n' || data[i] == '\r') continue;

        BinTree::Node *first = (BinTree::Node*) TableLookup(game->node_table, data + i);

        if (first == nullptr) {
            BinTreeInsert(&game->tree_.head_, data + i);

            first = game->tree_.head_;
            TableInsert(game->node_table, data + i, first);
        }

        while (data[i] != '>' && data[i] != '<') {
            ++i;
        }
        ++i;
        
        BinTree::Node *second = (BinTree::Node*) TableLookup(game->node_table, data + i);

        if (second == nullptr) {
            BinTreeInsert((data[i - 1] == '<' ? &first->L : &first->R), data + i);

            second = (data[i - 1] == '<' ? first->L : first->R);
            second->P = first;

            TableInsert(game->node_table, data + i, second);
        } else {
            (data[i - 1] == '<' ? first->L = second : first->R = second);
            second->P = first;
        }

        while (data[i] != '\0') {
            ++i;
        }
        ++i;
    }
}

static void SaveOldVersion() {
    #define OLD_TREE_VERSION_PATH "C:\\Users\\eleno\\AppData\\Local\\Temp\\akinator.tmp\\"
    #define OLD_TREE_VERSION_FILE "old_tree_version.txt"
    size_t kCommandBuferSize = 256;
    char buffer[kCommandBuferSize] = {};

    sprintf(buffer, "if not exist %s md %s && echo OLD AKINAOTR TREE VERSION > %s", OLD_TREE_VERSION_PATH, OLD_TREE_VERSION_PATH, OLD_TREE_VERSION_PATH OLD_TREE_VERSION_FILE);
    system(buffer);

    sprintf(buffer, "type %s >> %s", kSaveFileName, OLD_TREE_VERSION_PATH OLD_TREE_VERSION_FILE);
    system(buffer);

    sprintf(buffer, "echo ------------------- >> %s", OLD_TREE_VERSION_PATH OLD_TREE_VERSION_FILE);
    system(buffer);

    #undef OLD_TREE_VERSION_PATH
    #undef OLD_TREE_VERSION_FILE
}

static void WriteTreeToFile(Game *game, const char* file_name) {
    assert(game);
    assert(file_name);

    SaveOldVersion();

    FILE *save_file = nullptr;
    if (fopen_s(&save_file, file_name, "w")) {
        perror("Could not open the file for save tree\n");
        return;
    }

    Stack *stack = StackAllocate();
    if (game->tree_.head_) StackPush(stack, game->tree_.head_);
    while (StackSize(stack)) {
        BinTree::Node *node = StackPop(stack);

        if (node->L) {
            fprintf(save_file, "%s%c-<%s%c\n", (const char*)node->data, '\0', (const char*)node->L->data, '\0');
            StackPush(stack, node->L);
        }

        if (node->R) {
            fprintf(save_file, "%s%c->%s%c\n", (const char*)node->data, '\0', (const char*)node->R->data, '\0');
            StackPush(stack, node->R);
        }
    }

    fclose(save_file);
}

static void Init(Game *game) {
    assert(game);

    //game->state_ = Game::StartMenu;
    game->state_ = Game::ConsoleMode;

    game->node_table = TableAllocate();

    ReadTreeFromFile(game, kSaveFileName);

    //BinTreeInsert(&game->tree_.head_, strdup("Y or N"));
    //BinTreeInsert(&game->tree_.head_->L, strdup("YES"));
    //BinTreeInsert(&game->tree_.head_->R, strdup("NO"));
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

static void AddFork(Game *game) {
    assert(game);

    puts("What did you wish for?\n");

    const size_t kStrSize = 50;
    char name_buffer[kStrSize] = {};
    scanf("%s", name_buffer);
    BufferFlush();

    printf("\n%s differs from %s in that it is: ", name_buffer, GetDescription(game));

    char description_buffer[kStrSize] = {};
    scanf("%s", description_buffer);
    BufferFlush();

    BinTreeInsert(&game->active_node_->R, game->active_node_->data);
    BinTreeInsert(&game->active_node_->L, strdup(name_buffer));
    game->active_node_->data = strdup(description_buffer);

    game->active_node_->L->P = game->active_node_->R->P = game->active_node_;

    TableInsert(game->node_table, (const char*)game->active_node_->L->data, game->active_node_->L);
    TableInsert(game->node_table, (const char*)game->active_node_->R->data, game->active_node_->R);

    puts("\nOk, I'm remember\n");
}

static void GetPathFromRoot(BinTree::Node *node, Stack *path_stack) {
    assert(node);
    assert(path_stack);

    do {
        StackPush(path_stack, node);
        node = node->P;        
    } while (node->P);
}

static void TellNodeProperties(Game *game, const char* leaf) {
    assert(game);
    assert(leaf);

    BinTree::Node *node = (BinTree::Node*) TableLookup(game->node_table, leaf);
    if (!node) {
        printf("The selected sheet does not exist\n");
        return;
    }

    if (node->L || node->R) {
        printf("The selected sheet is not a sheet\n");
        return;
    }

    Stack *path_stack = StackAllocate();
    GetPathFromRoot(node, path_stack);

    printf("%s: ", leaf);

    while (StackSize(path_stack) - 1 > 0) {
        node = StackPop(path_stack);

        if (StackTop(path_stack) == node->R) {
            printf("!");
        }

        printf("%s ", (const char*)node->data);
    }
    
    printf("\n");
}

static void CreateGraphicsTree(Game *game) {
    assert(game);

    
}

static void ConsoleCicle(Game *game) {
    assert(game);

    TellNodeProperties(game, "apple");
    return;

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

    printf("It is: %s! Right? (Y/N)\n", GetDescription(game));
    char c = getc(stdin);
    BufferFlush();
    if (c == 'Y') {
        puts("I'm WINNNN!!!\n");
    } else if (c == 'N') {
        AddFork(game);
    } else {
        puts("Uncorrect answer :( ... I'm assume that i guessed right\n");
    }
}

static void MainLoop(Game *game) {
    assert(game);

    while (true) { // TODO:
        switch (game->state_) {
        case Game::ConsoleMode:
            ConsoleCicle(game);

            puts("You want to play again? (Y/N)\n");
            if (getc(stdin) == 'N') goto EXIT_MAIN_LOOP;
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

    EXIT_MAIN_LOOP:{}
    WriteTreeToFile(game, kSaveFileName);
}

// Game::public:
void RunGame(Game *game) {
    assert(game);

    Init(game);
    MainLoop(game);
}
