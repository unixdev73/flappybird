#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

namespace fb {
struct Application;

struct Button : public sf::Drawable {
private:
  bool hover_{false}, click_{false};

public:
  std::function<void(Application *, Button *)> onUnHover;
  std::function<void(Application *, Button *)> onHover;
  std::function<void(Application *, Button *)> onUnClick;
  std::function<void(Application *, Button *)> onClick;
  std::unique_ptr<sf::Text> text;
  sf::RectangleShape box;

  void draw(sf::RenderTarget &t, sf::RenderStates s) const override {
    t.draw(box, s);
    if (text)
      t.draw(*text, s);
  }

  void update(Application *a);
  void centerText();
};
} // namespace fb
