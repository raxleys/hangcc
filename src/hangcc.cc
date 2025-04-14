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

int main()
{
    auto images = parse_images(IMAGES_F.string());
    if (!images) {
        std::cerr << "Failed to parse images from: " << IMAGES_F.string() << '\n';
        return 1;
    }

    auto words = read_lines(WORDS_F);
    if (!words) {
        std::cerr << "Failed to read words from: " << WORDS_F.string() << '\n';
        return 1;
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
