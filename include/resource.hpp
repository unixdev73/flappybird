#pragma once

#include <SFML/Graphics.hpp>
#include <result.hpp>
#include <string>
#include <unordered_map>

namespace fb {

using TextureMap = std::unordered_map<std::string, sf::Texture>;
using FontMap = std::unordered_map<std::string, sf::Font>;

int ReadTexture(TextureMap *dst, const std::string &id,
                const std::string &path);
int ReadFont(FontMap *dst, const std::string &id, const std::string &path);

} // namespace fb
