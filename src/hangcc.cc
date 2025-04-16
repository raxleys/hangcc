////////////////////////////////////////////////////////////////////////////
// hangcc - Terminal based hangman game, written in C++                   //
// Copyright (C) 2025 Alexander Reyes <raxleys@gmail.com>                 //
//                                                                        //
// This program is free software: you can redistribute it and/or modify   //
// it under the terms of the GNU General Public License as published by   //
// the Free Software Foundation, either version 3 of the License, or      //
// (at your option) any later version.                                    //
//                                                                        //
// This program is distributed in the hope that it will be useful,        //
// but WITHOUT ANY WARRANTY; without even the implied warranty of         //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
// GNU General Public License for more details.                           //
//                                                                        //
// You should have received a copy of the GNU General Public License      //
// along with this program.  If not, see <https://www.gnu.org/licenses/>. //
////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <optional>
#include <vector>
#include <sstream>
#include <random>
#include <algorithm>
#include <cstdint>

namespace fs = std::filesystem;

// Constants
#define ASCII_RESET "\033[0m"
#define ASCII_GRAY  "\033[90m"
#define TERM_CLEAR  "\033[H\033[J"

const fs::path DATA_DIR  {"./data"};
const fs::path WORDS_F   {DATA_DIR / "words.txt"};
const fs::path IMAGES_F  {DATA_DIR / "images.txt"};

// Forward declarations
class Gameword;
std::optional<std::vector<std::string>> read_lines(const std::string& fname);
std::optional<std::vector<std::string>> parse_words(const std::string& fname);
std::optional<std::vector<std::string>> parse_images(const std::string& fname);
void print_alphabet(const Gameword& word);
void shuffle(std::vector<std::string>& words);

class Gameword
{
public:
    void set_word(std::string_view new_word)
    {
        word = new_word;
        guess_word.assign(new_word.size(), '_');
        guessed_letters = 0;
    }

    bool guess(char c)
    {
        c = std::toupper(c);
        bool correct = false;

        // Update guess_word
        for (size_t i = 0; i < word.size(); ++i) {
            if (word[i] == c) {
                guess_word[i] = c;
                correct = true;
            }
        }

        // Update set of guessed words
        guessed_letters |= (1 << (std::toupper(c) - 'A'));

        return correct;
    }

    bool already_guessed(char c) const
    {
        return guessed_letters & (1 << (std::toupper(c) - 'A'));
    }

    bool did_guess_word() const
    {
        return word == guess_word;
    }

    void print_guess(std::ostream& os) const
    {
        for (size_t i = 0; i < guess_word.size(); ++i) {
            os << guess_word[i];
            if (i != guess_word.size() - 1)
                os << ' ';
        }
        os << '\n';
    }

    std::string word{};
    std::string guess_word{};
    uint32_t guessed_letters{};
};

int main()
{
    auto oimages = parse_images(IMAGES_F.string());
    if (!oimages) {
        std::cerr << "Failed to parse images from: " << IMAGES_F.string() << '\n';
        return 1;
    }

    auto owords = parse_words(WORDS_F);
    if (!owords) {
        std::cerr << "Failed to read words from: " << WORDS_F.string() << '\n';
        return 1;
    }

    auto images = std::move(*oimages);
    auto words = std::move(*owords);
    size_t curr_word = words.size();
    size_t curr_im = 0;
    Gameword word;
    std::string input;
    std::string feedback_msg;
    bool new_game = true;
    bool game_over = false;
    while (true) {
        // Clear terminal
        std::cout << TERM_CLEAR;

        // Reshuffle word list if needed
        if (curr_word >= words.size()) {
            shuffle(words);
            curr_word = 0;
        }

        // Get next word
        if (new_game) {
            word.set_word(words[curr_word++]);
            new_game = false;
        }

        // Print image
        std::cout << images[curr_im] << "\n\n";

        // Print current guess
        std::cout << "Word: ";
        word.print_guess(std::cout);
        std::cout << '\n';

        // Print guessed letters
        print_alphabet(word);

        // Print error message, if any
        if (feedback_msg.size() > 0) {
            std::cout << "\n\n" << feedback_msg;
            feedback_msg.clear();
        }

        // Break out of loop here so that final image etc. is rendered on game loss
        if (game_over) {
            std::cout << "\nThe word was '" << word.word << "'\n";
            std::cout << "\nPlay again? (y/N): ";
            if (!std::getline(std::cin, input))
                break;

            if (std::toupper(input[0]) != 'Y')
                break;

            // Reset gamestate
            new_game = true;
            game_over = false;
            curr_im = 0;
            continue;
        }

        // Prompt user for a guess
        std::cout << "\n\nEnter a guess: ";
        if (!std::getline(std::cin, input))
            break;
        std::cout << '\n';

        if (input.size() != 1 || !std::isalpha(input[0])) {
            feedback_msg += "Invalid input!";
            continue;
        }

        char guess = input[0];
        if (word.already_guessed(guess)) {
            feedback_msg += "Letter was already guessed!";
            continue;
        }

        if (word.guess(guess)) {
            feedback_msg += "Correct!";
            if (word.did_guess_word()) {
                feedback_msg += "\n\nYou won!\n";
                game_over = true;
            }
        } else {
            feedback_msg += "Incorrect!";
            if (++curr_im >= images.size() - 1) {
                feedback_msg += "\n\nYou lost!\n";
                game_over = true;
            }
        }
    }

    return 0;
}

std::optional<std::vector<std::string>> read_lines(const std::string& fname)
{
    std::ifstream file(fname);
    if (!file) {
        std::cerr << "Failed to open file: " << fname << '\n';
        return std::nullopt;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);

    file.close();
    return lines;
}

std::optional<std::vector<std::string>> parse_words(const std::string& fname)
{
    auto words = read_lines(fname);
    if (!words.has_value())
        return words;

    for (auto& word : *words) {
        for (size_t i = 0; i < word.size(); i++)
            word[i] = std::toupper(word[i]);
    }

    return words;
}

std::optional<std::vector<std::string>> parse_images(const std::string& fname)
{
    std::ifstream file(fname);
    if (!file) {
        std::cerr << "Failed to open file: " << fname << '\n';
        return std::nullopt;
    }

    std::ostringstream fbuf;
    fbuf << file.rdbuf();
    std::string contents = fbuf.str();
    file.close();

    std::vector<std::string> images;
    std::ostringstream oss;
    for (size_t i = 0; i < contents.size(); ++i) {
        if (contents[i] != ',') {
            oss.put(contents[i]);
        } else {
            images.push_back(std::move(oss.str()));
            oss.str("");
            oss.clear();

            // Consume newlines
            for (++i; i < contents.size(); ++i) {
                if (contents[i] == '\r' || contents[i] == '\n')
                    continue;
                break;
            }
            --i;
        }
    }

    return images;
}

void print_alphabet(const Gameword& word)
{
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (word.already_guessed(c)) {
            std::cout << ASCII_GRAY;
        }

        std::cout << c << ASCII_RESET;

        if (c != 'Z' || (c - 'A') % 10 != 0)
            std::cout << ' ';

        if (c != 'A' && (c - 'A') % 10 == 0)
            std::cout << '\n';
    }
}

void shuffle(std::vector<std::string>& words)
{
     static std::random_device rd;
     static std::mt19937 g(rd());

     std::shuffle(words.begin(), words.end(), g);
}
