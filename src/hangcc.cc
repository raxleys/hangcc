#include <iostream>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <optional>
#include <vector>
#include <sstream>
#include <random>
#include <algorithm>

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

    void guess(char c)
    {
        for (size_t i = 0; i < word.size(); ++i) {
            if (word[i] == c) {
                guess_word[i] = c;
            }
        }
    }

    bool already_guessed(char c)
    {
        return guess_word.find(std::toupper(c)) != std::string::npos;
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
    std::string error_msg;
    do {
        // Reshuffle word list if needed
        if (curr_word >= words.size()) {
            shuffle(words);
            curr_word = 0;
        }

        // Get next word
        word.set_word(words[curr_word++]);

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
        if (error_msg.size() > 0) {
            std::cout << "\n\n" << error_msg << "\n";
            error_msg.clear();
        }

        // Prompt user for a guess
        std::cout << "\nEnter a guess: ";
        std::getline(std::cin, input);
        std::cout << "READ: '" << input << "'\n";
        std::cout << input.size() << '\n';

        if (input.size() != 1 || !std::isalpha(input[0])) {
            error_msg += "Invalid input!";
            continue;
        }

        char guess = input[0];
        if (word.already_guessed(guess)) {
            error_msg += "Letter was already guessed!";
            continue;
        }

        std::cout << "GUESS: " << guess;
    } while (true);

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
