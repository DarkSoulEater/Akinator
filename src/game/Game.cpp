#define TX_USE_SPEAK
#include "C:\src\TX\TXLib.h"
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
            BinTreeInsert((data[i - 1] == '<' ? &first->left_ : &first->right_), data + i);

            second = (data[i - 1] == '<' ? first->left_ : first->right_);
            second->parent_ = first;

            TableInsert(game->node_table, data + i, second);
        } else {
            (data[i - 1] == '<' ? first->left_ = second : first->right_ = second);
            second->parent_ = first;
        }

        while (data[i] != '\0') {
            ++i;
        }
        ++i;
    }
}

static void SaveOldVersion() { //TODO: getenv("temp"); // TODO: Add ~akinator
    #define OLD_TREE_VERSION_PATH "C:\\Users\\eleno\\AppData\\Local\\Temp\\akinator.tmp\\"
    #define OLD_TREE_VERSION_FILE "old_tree_version.txt"
    const size_t kCommandBuferSize = 256;
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

        if (node->left_) {
            fprintf(save_file, "%s%c-<%s%c\n", (const char*)node->data_, '\0', (const char*)node->left_->data_, '\0');
            StackPush(stack, node->left_);
        }

        if (node->right_) {
            fprintf(save_file, "%s%c->%s%c\n", (const char*)node->data_, '\0', (const char*)node->right_->data_, '\0');
            StackPush(stack, node->right_);
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

    return game->active_node_->left_ == nullptr && game->active_node_->right_ == nullptr;
}

static const char* GetQuestion(Game *game) {\
    assert(game);
    assert(game->active_node_);

    return (const char*)game->active_node_->data_;
}

static const char* GetDescription(Game *game) {
    assert(game);
    assert(game->active_node_);

    return (const char*)game->active_node_->data_;
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
    scanf("%[^\n]", name_buffer);
    BufferFlush();

    printf("\n%s differs from %s in that it is: ", name_buffer, GetDescription(game));

    char description_buffer[kStrSize] = {};
    scanf("%[^\n]", description_buffer);
    BufferFlush();

    BinTreeInsert(&game->active_node_->right_, game->active_node_->data_);
    BinTreeInsert(&game->active_node_->left_, strdup(name_buffer));
    game->active_node_->data_ = strdup(description_buffer);

    game->active_node_->left_->parent_ = game->active_node_->right_->parent_ = game->active_node_;

    TableInsert(game->node_table, (const char*)game->active_node_->left_->data_, game->active_node_->left_);
    TableInsert(game->node_table, (const char*)game->active_node_->data_, game->active_node_);

    BinTree::Node **data_ptr = (BinTree::Node**)TableGetDataPtr(game->node_table, (const char*)game->active_node_->right_->data_);
    if (data_ptr == nullptr) {
        fprintf(stderr, "error: vertex data was not created in hash-table\n");
        abort();
    }
    *data_ptr = game->active_node_->right_;

    puts("\nOk, I'm remember\n");
}

static void GetPathFromRoot(BinTree::Node *node, Stack *path_stack) {
    assert(node);
    assert(path_stack);

    do {
        StackPush(path_stack, node);
        node = node->parent_;        
    } while (node);
}

static void TellNodeProperties(Game *game, const char* leaf) {
    assert(game);
    assert(leaf);

    BinTree::Node *node = (BinTree::Node*) TableLookup(game->node_table, leaf);
    if (!node) {
        printf("The selected sheet does not exist\n");
        return;
    }

    if (node->left_ || node->right_) {
        printf("The selected sheet is not a sheet\n");
        return;
    }

    Stack *path_stack = StackAllocate();
    GetPathFromRoot(node, path_stack);

    printf("%s: ", leaf);

    while (StackSize(path_stack) - 1 > 0) {
        node = StackPop(path_stack);

        if (StackTop(path_stack) == node->right_) {
            printf("!");
        }

        printf("%s ", (const char*)node->data_);
    }
    
    printf("\n");
}

static void TellNodeComparison(Game *game, const char* one_leaf, const char* two_leaf) {
    assert(game);
    assert(one_leaf);
    assert(two_leaf);

    BinTree::Node *one_node = (BinTree::Node*) TableLookup(game->node_table, one_leaf);
    if (!one_node) {
        printf("The 1 selected sheet does not exist\n");
        return;
    }

    if (one_node->left_ || one_node->right_) {
        printf("The 1 selected sheet is not a sheet\n");
        return;
    }

    BinTree::Node *two_node = (BinTree::Node*) TableLookup(game->node_table, two_leaf);
    if (!two_node) {
        printf("The 2 selected sheet does not exist\n");
        return;
    }

    if (two_node->left_ || two_node->right_) {
        printf("The 2 selected sheet is not a sheet\n");
        return;
    }

    Stack *one_path_stack = StackAllocate();
    GetPathFromRoot(one_node, one_path_stack);

    Stack *two_path_stack = StackAllocate();
    GetPathFromRoot(two_node, two_path_stack);

    txSpeak(" e, boy .snake and Denchick similar to those: animal, but snake is:  not came from hell, creeps and Denchick is: chill room, not sleeps on the right, down ... say thank you pathetic man...You want to play again? (Y/N)");
    return;

    txSpeak("\vi found the tops but first rate the rap. BschBschchtttyyytttyyytttyyy, e, boy");

    txSpeak("\v%s and %s similar to those: ", one_leaf, two_leaf);

    while (StackSize(one_path_stack) - 1 > 0 && StackSize(two_path_stack) - 1 > 0) {
        one_node = StackPop(one_path_stack);
        two_node = StackPop(two_path_stack);

        BinaryTree::Node *one_node_next = (StackTop(one_path_stack) == one_node->left_ ? 
                                            one_node->left_ : one_node->right_);

        BinaryTree::Node *two_node_next = (StackTop(two_path_stack) == two_node->left_ ? 
                                            two_node->left_ : two_node->right_);

        if (one_node == two_node) {
            if (StackTop(one_path_stack) == one_node->right_) {
                txSpeak("\vnot ");
            }
            txSpeak("\v%s, ", (const char*)one_node->data_);

        } else {
            StackPush(one_path_stack, one_node);
            StackPush(two_path_stack, two_node);

            break;
        }
    }

    txSpeak("\vbut %s is: ", one_leaf);
    while (StackSize(one_path_stack) - 1 > 0) {
        one_node = StackPop(one_path_stack);

        if (StackTop(one_path_stack) == one_node->right_) {
            txSpeak("\v not ");
        }
        txSpeak("\v%s", (const char*)one_node->data_);
    }

    txSpeak("\v\n and %s is: ", two_leaf);
    while (StackSize(two_path_stack) - 1 > 0) {
        two_node = StackPop(two_path_stack);
        
        if (StackTop(two_path_stack) == two_node->right_) {
            txSpeak("\v not ");
        }
        txSpeak("\v%s", (const char*)two_node->data_);
    }

    txSpeak("\v ...say thank you pathetic man...");
}

#define DOT_PATH "\"C:/Program Files/Graphviz/bin/dot.exe\""
static void CreateGraphicsTree(Game *game) {
    assert(game);

    const char* kTreeDumpPath = "bd/tree.dot";

    FILE *graph_file = nullptr;
    if (fopen_s(&graph_file, kTreeDumpPath, "w")) {
        perror("Could not open file\n");
        return;
    }

    fprintf(graph_file, "digraph G{\nsearchsize=100\n");

    Stack *dfs_stack = StackAllocate();
    BinTree::Node *node = game->tree_.head_;
    StackPush(dfs_stack, node);

    while (StackSize(dfs_stack)){
        node = StackPop(dfs_stack);

        if (node->left_) {
            fprintf(graph_file, "\"%s\"->\"%s\";\n", (const char*)node->data_, (const char*)node->left_->data_);
            StackPush(dfs_stack, node->left_);
        }

        if (node->right_) {
            fprintf(graph_file, "\"%s\"->\"%s\";\n", (const char*)node->data_, (const char*)node->right_->data_);
            StackPush(dfs_stack, node->right_);
        }

        fprintf(graph_file, "\"%s\"[color=black];\n", (const char*)node->data_);
    }

    fprintf(graph_file, "}");

    fclose(graph_file);

    char command_buffer[256] = {};
    sprintf(command_buffer, DOT_PATH  " -O %s -T png", kTreeDumpPath);
    system(command_buffer);

    system("bd\\tree.dot.png");
}

static void AkinatorGame(Game *game) {
    game->active_node_ = game->tree_.head_;

    while (!ActiveNodeIsTerminal(game)) {
        printf("It is %s? (Y/N)\n", GetQuestion(game));
        
        char c = 0;
        if ((c = (char)getc(stdin)) == 'Y') {
            game->active_node_ = game->active_node_->left_;
        } else if (c == 'N'){
            game->active_node_ = game->active_node_->right_;
        } else {
            puts("Uncorrect answer :(\n");
        }

        BufferFlush();
    }

    printf("It is: %s! Right? (Y/N)\n", GetDescription(game));
    char c = (char)getc(stdin);
    BufferFlush();
    if (c == 'Y') {
        puts("I'm WINNNN!!!\n");
    } else if (c == 'N') {
        AddFork(game);
    } else {
        puts("Uncorrect answer :( ... I'm assume that i guessed right\n");
    }
}

static void ConsoleCicle(Game *game) {
    assert(game);

    puts("Select game: [a]kinator | [p]roperties | [c]omparison | [g]raphic\n");

    char cur = (char)getc(stdin);
    BufferFlush();

    switch (cur) {
    case 'a': {
        AkinatorGame(game);
    } break;
    
    case 'p': {
        char name[100] = {};
        scanf("%[^\n]", name);
        BufferFlush();

        TellNodeProperties(game, name);   
    } break;

    case 'c': {
        char name_1[100] = "";
        scanf("%[^\n]", name_1);
        BufferFlush();

        char name_2[100] = "";
        scanf("%[^\n]", name_2);
        BufferFlush();

        TellNodeComparison(game, name_1, name_2);

    } break;

    case 'g': {
        CreateGraphicsTree(game);
    } break;

    default:{
        printf("Uncorrect game type\n");
    } break;

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
