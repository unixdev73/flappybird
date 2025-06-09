#include "result.hpp"
#include <filesystem>
#include <iostream>
#include <resource.hpp>

namespace fs = std::filesystem;

namespace {
template <typename T>
int ValidateResourceParameters(std::unordered_map<std::string, T> *dst,
                               const std::string &id, const std::string &path) {

  if (!dst) {
    std::cerr << "(ERR): The resource destination is a nullptr!" << std::endl;
    return fb::Result::DomainError;
  }

  if (!id.size()) {
    std::cerr << "(ERR): The resource id cannot have zero length!" << std::endl;
    return fb::Result::DomainError;
  }

  if (dst->contains(id)) {
    std::cerr << "(ERR): The resource id: '" << id << "' is already in use!"
              << std::endl;
    return fb::Result::DomainError;
  }

  if (!path.size() || !fs::exists(path)) {
    std::cerr << "(ERR): The resource path: '" << path << "' is not valid!"
              << std::endl;
    return fb::Result::DomainError;
  }

  return fb::Result::Success;
}
} // namespace

namespace fb {
int ReadTexture(TextureMap *dst, const std::string &id,
                const std::string &path) {
  if (auto r = ValidateResourceParameters(dst, id, path); r != Result::Success)
    return r;

  sf::Texture t;
  if (!t.loadFromFile(path)) {
    std::cerr << "(ERR): Failed to load texture: '" << path << std::endl;
    return Result::ReadError;
  }

  dst->emplace(id, std::move(t));
  return Result::Success;
}

int ReadFont(FontMap *dst, const std::string &id, const std::string &path) {
  if (auto r = ValidateResourceParameters(dst, id, path); r != Result::Success)
    return r;

  sf::Font f;
  if (!f.openFromFile(path)) {
    std::cerr << "(ERR): Failed to load texture: '" << path << std::endl;
    return Result::ReadError;
  }

  dst->emplace(id, std::move(f));
  return Result::Success;
}
} // namespace fb
