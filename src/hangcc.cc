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
const fs::path DATA_DIR  {"./data"};
const fs::path WORDS_F   {DATA_DIR / "words.txt"};
const fs::path IMAGES_F  {DATA_DIR / "images.txt"};

// Forward declarations
std::optional<std::vector<std::string>> read_lines(const std::string& fname);
std::optional<std::vector<std::string>> parse_words(const std::string& fname);
std::optional<std::vector<std::string>> parse_images(const std::string& fname);
void shuffle(std::vector<std::string>& words);

class Gameword
{
public:
    void set_word(std::string_view new_word)
    {
        // TODO: remove
        std::cerr << "[DEBUG] word = " << new_word << '\n';
        word = new_word;
        guess_word.assign(new_word.size(), '_');
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

    bool already_guessed(char c)
    {
        return guessed_letters & (1 << (std::toupper(c) - 'A'));
    }

    bool did_guess_word()
    {
        return word == guess_word;
    }

    void print_guess(std::ostream& os)
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
    while (true) {
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
        // TODO: ASCII control codes
        for (char c = 'A'; c <= 'Z'; ++c) {
            std::cout << c;

            if (c != 'Z' || (c - 'A') % 10 != 0)
                std::cout << ' ';

            if (c != 'A' && (c - 'A') % 10 == 0)
                std::cout << '\n';
        }

        // Print error message, if any
        if (feedback_msg.size() > 0) {
            std::cout << "\n\n" << feedback_msg;
            feedback_msg.clear();
        }

        // Prompt user for a guess
        std::cout << "\n\nEnter a guess: ";
        if (!std::getline(std::cin, input))
            break;

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
            // TODO: Win condition
        } else {
            feedback_msg += "Incorrect!";
            if (++curr_im >= images.size() - 1) {
                std::cout << "\nYou lost!\n";
                break;
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

void shuffle(std::vector<std::string>& words)
{
     static std::random_device rd;
     static std::mt19937 g(rd());

     std::shuffle(words.begin(), words.end(), g);
}
