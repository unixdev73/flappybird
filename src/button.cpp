#include <application.hpp>
#include <button.hpp>

namespace fb {
void Button::centerText() {
  auto tx = this->text.get();
  if (!tx)
    return;

  auto bg = &this->box;

  tx->setPosition({bg->getPosition().x +
                       (bg->getSize().x - tx->getLocalBounds().size.x) / 2.f -
                       tx->getLocalBounds().position.x,
                   bg->getPosition().y +
                       (bg->getSize().y - tx->getLocalBounds().size.y) / 2.f -
                       tx->getLocalBounds().position.y});
}

void Button::update(Application *a) {
  const sf::Vector2f pos{GetMousePositionX(a), GetMousePositionY(a)};
  const sf::FloatRect bb{box.getGlobalBounds()};

  if (bb.contains(pos)) {
    if (!IsButtonHovered(a)) {
      SetButtonHovered(a, true);
      hover_ = true;
      if (this->onHover)
        this->onHover(a, this);
    }
  } else if (hover_) {
    SetButtonHovered(a, false);
    hover_ = false;
    if (this->onUnHover)
      this->onUnHover(a, this);
  }

  if (IsPrimaryMouseButtonPressed(a)) {
    if (hover_ && !IsButtonClicked(a)) {
      SetButtonClicked(a, true);
      click_ = true;
      if (this->onClick)
        this->onClick(a, this);
    }
  } else if (click_) {
    SetButtonClicked(a, false);
    click_ = false;
    if (this->onUnClick && hover_)
      this->onUnClick(a, this);
  }
}
} // namespace fb
