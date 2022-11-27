#include <vector>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <utility>

namespace fs = std::filesystem;

fs::path filepath;

enum class Action { Cut, Copy, Paste };
Action action;

std::vector<fs::path> items;

unsigned int files_success = 0;
unsigned int directories_success = 0;

void displayHelpMessage() {
    printf("\033[38;5;51m▏This is Clipboard 0.1.0, the copy and paste system for the command line.\033[0m\n");
    printf("\033[38;5;51m\033[1m▏How To Use\033[0m\n");
    printf("\033[38;5;208m▏clipboard cut [options] (item) [items]\033[0m\n");
    printf("\033[38;5;208m▏clipboard copy [options] (item) [items]\033[0m\n");
    printf("\033[38;5;208m▏clipboard paste [options]\033[0m\n");
    printf("\033[38;5;51m▏You can substitute \"cb\" for \"clipboard\" to save time.\033[0m\n");
    printf("\033[38;5;51m\033[1m▏Examples\033[0m\n");
    printf("\033[38;5;208m▏cb cut nuclearlaunchcodes.txt Contacts_Folder\033[0m\n");
    printf("\033[38;5;208m▏clipboard copy dogfood.conf\033[0m\n");
    printf("\033[38;5;208m▏cb paste\033[0m\n");
    printf("\033[38;5;51m▏Copyright (C) 2022 Jackson Huff. Licensed under the GPLv3.\033[0m\n");
    printf("\033[38;5;51m▏This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions.\033[0m\n");
}

void setupVariables(const int argc, char *argv[]) {
    filepath = fs::temp_directory_path() / "Clipboard";

    for (int i = 2; i < argc; i++) {
        items.emplace_back(argv[i]);
    }
}

void checkFlags(const int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            displayHelpMessage();
            exit(0);
        }
    }
}

void setupAction(const int argc, char *argv[]) {
    if (argc >= 2) {
        if (!strcmp(argv[1], "cut")) {
            action = Action::Cut;
        } else if (!strcmp(argv[1], "copy")) {
            action = Action::Copy;
        } else if (!strcmp(argv[1], "paste")) {
            action = Action::Paste;
        } else {
            printf("\033[38;5;196m╳ You did not specify a valid action, or you forgot to include one. \033[38;5;219mTry using or adding \033[1mcut, copy, or paste\033[0m\033[38;5;219m instead, like \033[1mclipboard copy\033[0m.\n");
            exit(1);
        }
    } else {
        printf("\033[38;5;196m╳ You did not specify an action. \033[38;5;219mTry adding \033[1mcut, copy, or paste\033[0m\033[38;5;219m to the end, like \033[1mclipboard copy\033[0m\033[38;5;219m. If you need more help, try \033[1mclipboard -h\033[0m\033[38;5;219m to show the help screen.\033[0m\n");
        exit(1);
    }
}

void checkForNoItems() {
    if ((action != Action::Paste) && items.size() < 1) {
        if (action == Action::Copy) {
            printf("\033[38;5;196m╳ You need to choose something to copy.\033[38;5;219m Try adding the items you want to copy to the end, like \033[1mcopy contacts.txt myprogram.cpp\033[0m\n");
        } else if (action == Action::Cut) {
            printf("\033[38;5;196m╳ You need to choose something to cut.\033[38;5;219m Try adding the items you want to cut to the end, like \033[1mcut contacts.txt myprogram.cpp\033[0m\n");
        }
        exit(1);
    }
}

void setupTempDirectory() {
    if (fs::is_directory(filepath)) {
        if (action != Action::Paste) {
            for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
                fs::remove_all(entry.path());
            }
        }
    } else {
        fs::create_directories(filepath);
    }
}

void setupIndicator() {
    if (action == Action::Copy) {
        printf("\033[38;5;214m• Copying...\033[0m\r");
    } else if (action == Action::Cut) {
        printf("\033[38;5;214m• Cutting...\033[0m\r");
    } else if (action == Action::Paste) {
        printf("\033[38;5;214m• Pasting...\033[0m\r");
    }
    fflush(stdout);
}

void performAction() {
    std::vector<std::pair<fs::path, fs::filesystem_error>> failedItems;
    fs::copy_options opts = fs::copy_options::recursive | fs::copy_options::copy_symlinks | fs::copy_options::overwrite_existing;
    if (action == Action::Copy) {
        for (const auto& f : items) {
            try {
                if (fs::is_directory(f)) {
                    fs::create_directories(filepath / f.parent_path().filename());
                    fs::copy(f, filepath / f.parent_path().filename(), opts);
                } else {
                    fs::copy(f, filepath / f.filename(), opts);
                }
            } catch (const fs::filesystem_error& e) {
                failedItems.emplace_back(f, e);
            }
        }
    } else if (action == Action::Cut) {
        for (const auto& f : items) {
            try {
                if (fs::is_directory(f)) {
                    fs::create_directories(filepath / f.parent_path().filename());
                    fs::rename(f, filepath / f.parent_path().filename());
                } else {
                    fs::rename(f, filepath / f.filename());
                }
            } catch (const fs::filesystem_error& e) {
                failedItems.emplace_back(f, e);
            }
        }  
    } else if (action == Action::Paste) {
        try {
            fs::copy(filepath, fs::current_path(), opts);
            printf("\033[38;5;40m√ Pasted\033[0m");
        } catch (const fs::filesystem_error& e) {
            printf("\033[38;5;196m╳ Failed to paste\033[0m");
        }
    }
    if (failedItems.size() > 0) {
        printf("\033[38;5;196m╳ Clipboard couldn't %s these items.\033[0m\n", action == Action::Copy ? "copy" : "cut");
        for (int i = 0; i < std::min(5, int(failedItems.size())); i++) {
            printf("\033[38;5;196m▏ %s: %s\033[0m\n", failedItems.at(i).first.string().data(), failedItems.at(i).second.code().message().data());
            if (i == 4 && failedItems.size() > 5) {
                printf("\033[38;5;196m▏ ...and %d more.\033[0m\n", int(failedItems.size()) - 5);
            }
        }
        printf("\033[38;5;219m▏ See if you have the needed permissions, or\033[0m\n");
        printf("\033[38;5;219m▏ try double-checking the spelling of the files or what directory you're in.\033[0m\n");
    }
    for (const auto& f : failedItems) {
        items.erase(std::remove(items.begin(), items.end(), f.first), items.end());
    }
}

void countSuccessesAndFailures() {
    if (action == Action::Copy) {
        for (const auto& f : items) {
            if (fs::is_directory(f)) {
                if (fs::exists(filepath / f.parent_path().filename())) {
                    directories_success++;
                }
            } else {
                if (fs::exists(filepath / f.filename())) {
                    files_success++;
                }
            }
        }
    } else if (action == Action::Cut) {
        for (const auto& f : items) {
            if (fs::is_directory(filepath / f.parent_path().filename())) {
                if (fs::exists(filepath / f.parent_path().filename())) {
                    directories_success++;
                }
            } else {
                if (fs::exists(filepath / f.filename())) {
                    files_success++;
                }
            }
        }
    }
}

void showSuccesses() {
    if ((files_success >= 1) != (directories_success >= 1)) {
        if (action == Action::Copy) {
            printf("\033[38;5;40m√ Copied %s\033[0m\n", items.at(0).string().data());
        } else if (action == Action::Cut) {
            printf("\033[38;5;40m√ Cut %s\033[0m\n", items.at(0).string().data());
        }
    } else {
        if ((files_success > 1) && (directories_success == 0)) {
            if (action == Action::Copy) {
                printf("\033[38;5;40m√ Copied %i files\033[0m\n", files_success);
            } else if (action == Action::Cut) {
                printf("\033[38;5;40m√ Cut %i files\033[0m\n", files_success);
            }
        } else if ((files_success == 0) && (directories_success > 1)) {
            if (action == Action::Copy) {
                printf("\033[38;5;40m√ Copied %i directories\033[0m\n", directories_success);
            } else if (action == Action::Cut) {
                printf("\033[38;5;40m√ Cut %i directories\033[0m\n", directories_success);
            }
        } else if ((files_success >= 1) && (directories_success >= 1)) {
            if (action == Action::Copy) {
                printf("\033[38;5;40m√ Copied %i files and %i directories\033[0m\n", files_success, directories_success);
            } else if (action == Action::Cut) {
                printf("\033[38;5;40m√ Cut %i files and %i directories\033[0m\n", files_success, directories_success);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        setupVariables(argc, argv);

        checkFlags(argc, argv);

        setupAction(argc, argv);

        checkForNoItems();

        setupIndicator();

        setupTempDirectory();

        performAction();

        countSuccessesAndFailures();

        showSuccesses();
    } catch (const std::exception& e) {
        printf("\033[38;5;196m╳ Internal error: %s\n▏ This is probably a bug.\033[0m\n", e.what());
        exit(1);
    }
    return 0;
}