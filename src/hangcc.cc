#include <iostream>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <optional>
#include <vector>
#include <sstream>

namespace fs = std::filesystem;

// Constants
const fs::path DATA_DIR  {"./data"};
const fs::path WORDS_F   {DATA_DIR / "words.txt"};
const fs::path IMAGES_F  {DATA_DIR / "images.txt"};

// Forward declarations
std::optional<std::vector<std::string>> read_lines(const std::string& fname);
std::optional<std::vector<std::string>> parse_images(const std::string& fname);

class Gameword
{
public:
    void set_word(std::string_view new_word)
    {
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

    bool was_guessed()
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

    auto owords = read_lines(WORDS_F);
    if (!owords) {
        std::cerr << "Failed to read words from: " << WORDS_F.string() << '\n';
        return 1;
    }

    auto images = std::move(*oimages);
    auto words = std::move(*owords);
    size_t curr_im = 0;
    // TODO: Make random word.
    Gameword word;
    word.set_word(words[0]);
    do {
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

        // Prompt user for a guess
        std::cout << "\n\nEnter a guess: ";
        std::cout << std::endl;

    } while (0);

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
