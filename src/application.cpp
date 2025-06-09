#include <SFML/Window/Keyboard.hpp>
#include <animation.hpp>
#include <application.hpp>
#include <button.hpp>
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <list>
#include <random>
#include <ranges>
#include <regex>
#include <resource.hpp>
#include <result.hpp>
#include <scene.hpp>
#include <string>
#include <thread>
#include <vector>

namespace {
void SetCurrentWorkingDirectory(const char *bin) {
  namespace fs = std::filesystem;
  fs::current_path(fs::absolute(fs::path{bin}.parent_path()));
}

template <typename T>
T GetTimeSince(const std::chrono::time_point<std::chrono::steady_clock> &m) {
  return std::chrono::duration_cast<T>(std::chrono::steady_clock::now() - m);
}

template <typename T>
int ExtractParameterValue(int argc, char **argv, std::string regex, T *dst);

int Update(fb::Application *a);

void CreateWindow(fb::Application *a, int c, char **v);
} // namespace

namespace fb {
struct MainMenu : public Scene {
public:
  MainMenu(Application *ptr) : Scene(ptr) {}
  int build() override;
  int update() override;
  int render() override;
  int clear() override;

private:
  std::list<Button> buttons_;
  FontMap fonts_;
};

struct Bird : public sf::Drawable {
  std::unique_ptr<fb::Animation, int (*)(Animation *)> animation;
  std::unique_ptr<sf::Sprite> body;

  void draw(sf::RenderTarget &r, sf::RenderStates s) const override {
    r.draw(*body, s);
  }

  Bird() : animation{nullptr, fb::DestroyAnimation} {}

  void update(Application *, bool animate);
};

struct Fence : public sf::Drawable {
  sf::RectangleShape body;
  float up_{1};
  float v_{2};
  bool score_{true};

  void update(Application *, unsigned maxSpeed);

  void draw(sf::RenderTarget &r, sf::RenderStates s) const override {
    r.draw(body, s);
  }
};

struct InGame : public Scene {
public:
  InGame(Application *ptr) : Scene(ptr) {}
  int build() override;
  int update() override;
  int render() override;
  int clear() override;

  unsigned scoreCount_{0};
  Button *score_;

private:
  std::list<Button> buttons_;
  std::list<Fence> fences_;
  TextureMap textures_;
  FontMap fonts_;

  Bird bird_;

  bool gameOver_{false};
  bool launch_{false};

  const float dv_{9.81};
  float v_{0};

  sf::RectangleShape bg_;
};

struct Application {
  std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;

  sf::RenderWindow window;

  Scene *active;

  /* The minimum duration of processing one frame */
  std::chrono::milliseconds minTPF;

  /* The processing time of the previous frame */
  std::chrono::milliseconds elapsed;

  sf::Vector2f mousePos;

  bool primaryMouseButtonPressed{false};
  bool buttonClicked{false};
  bool buttonHovered{false};
  bool flapRequested{false};

  using Command = std::function<void(Application *)>;
  std::list<Command> commandQ;

  static inline std::mt19937 rng{std::random_device{}()};
};

int Initialize(Application *&app, int argc, char **argv) {
  SetCurrentWorkingDirectory(argv[0]);
  app = new Application{};

  unsigned tpf = 16; // This translates to about 60 FPS
  ExtractParameterValue(argc, argv, "--time-per-frame|-t", &tpf);
  app->minTPF =
      std::chrono::milliseconds{tpf}; // This controls the framerate
                                      // by specifying that the minimum
                                      // frame duration should be 17ms

  CreateWindow(app, argc, argv);

  app->scenes.emplace("MainMenu", new MainMenu{app});
  app->scenes.emplace("InGame", new InGame{app});

  for (auto &&s : app->scenes)
    if (auto r = s.second->build(); r != Result::Success) {
      auto msg = "Failed to build scene: " + s.first + " with error code: ";
      LogErr(msg.c_str(), r);
      return r;
    }

  ScheduleSceneTransition(app, "MainMenu");
  return Result::Success;
}

int Run(Application *a) {
  auto p = std::chrono::steady_clock::now();

  while (a->window.isOpen()) {
    if (auto e = GetTimeSince<decltype(a->minTPF)>(p); e < a->minTPF) {
      std::this_thread::sleep_for(std::chrono::microseconds{100});
      continue;
    } else
      a->elapsed = e;
    p = std::chrono::steady_clock::now();

    while (auto event = a->window.pollEvent())
      if (event->is<sf::Event::Closed>())
        a->window.close();

    if (auto r = Update(a); r != Result::Success) {
      LogErr("The main update function failed with error code: ", r);
      return r;
    }
  }

  return Result::Success;
}

void Destroy(Application *a) { delete a; }

void Render(Application *a, sf::Drawable *d) { a->window.draw(*d); }

void LogErr(const char *m, int n) {
  std::cerr << "(ERR): " << m << n << std::endl;
}

void LogErr(const char *m) { std::cerr << "(ERR): " << m << std::endl; }

double GetFrameTimeInSeconds(Application *a) {
  auto e = std::chrono::duration_cast<std::chrono::milliseconds>(a->elapsed);
  auto secPerMs = 0.001;
  return e.count() * secPerMs;
}

float GetMousePositionX(Application *a) { return a->mousePos.x; }
float GetMousePositionY(Application *a) { return a->mousePos.y; }
float GetWindowSizeX(Application *a) { return a->window.getSize().x; }
float GetWindowSizeY(Application *a) { return a->window.getSize().y; }

bool IsPrimaryMouseButtonPressed(Application *a) {
  return a->primaryMouseButtonPressed;
}
void SetButtonClicked(Application *a, bool v) { a->buttonClicked = v; }
bool IsButtonClicked(Application *a) { return a->buttonClicked; }
void SetButtonHovered(Application *a, bool v) { a->buttonHovered = v; }
bool IsButtonHovered(Application *a) { return a->buttonHovered; }

void IncrementScore(Application *a) {
  auto scene = dynamic_cast<InGame *>(a->scenes.at("InGame").get());
  scene->score_->text->setString("Score: " +
                                 std::to_string(++scene->scoreCount_));
}

unsigned GetScore(Application *a) {
  auto scene = dynamic_cast<InGame *>(a->scenes.at("InGame").get());
  return scene->scoreCount_;
}

unsigned GetRandomNumber(Application *a, unsigned inclBegin, unsigned exclEnd) {
  return inclBegin + a->rng() % exclEnd;
}

bool IsFlapRequested(Application *) {
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
    return true;
  return false;
}

void ScheduleExit(Application *a) {
  a->commandQ.push_back([](Application *app) { app->window.close(); });
}

void ScheduleSceneTransition(Application *a, const char *scene) {
  a->commandQ.push_back([scene](Application *app) {
    auto target = app->scenes.at(scene).get();
    if (target->requiresRebuild()) {
      target->build();
      target->requiresRebuild(false);
    }
    app->active = target;
    app->primaryMouseButtonPressed = false;
    app->buttonClicked = false;
    app->buttonHovered = false;
  });
}

void ScheduleSceneClear(Application *app) {
  if (app->active)
    app->commandQ.push_back([](Application *a) {
      a->active->clear();
      a->active->requiresRebuild(true);
    });
}
} // namespace fb

namespace {
void CreateWindow(fb::Application *a, int c, char **v) {
  const auto d = sf::VideoMode::getDesktopMode();
  sf::Vector2u s{960, 540};
  unsigned w, h;
  ExtractParameterValue(c, v, "--width|-w", &w);
  ExtractParameterValue(c, v, "--height|-h", &h);
  sf::VideoMode m{
      {w > s.x && w <= d.size.x ? w : s.x, h > s.y && h < d.size.y ? h : s.y}};
  a->window.create(m, "Flappy Bird", sf::Style::Close);
}

void UpdateMouseInfo(fb::Application *a) {
  if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    a->primaryMouseButtonPressed = true;
  else
    a->primaryMouseButtonPressed = false;

  const auto mPos = sf::Mouse::getPosition(a->window);
  a->mousePos.x = mPos.x;
  a->mousePos.y = mPos.y;
}

int Update(fb::Application *a) {
  UpdateMouseInfo(a);

  if (fb::Scene *s = a->active; s && !s->requiresRebuild()) {
    if (auto r = s->update(); r != fb::Result::Success) {
      fb::LogErr("Failed to update active scene with error code: ", r);
      return r;
    }

    a->window.clear();
    if (auto r = s->render(); r != fb::Result::Success) {
      fb::LogErr("Failed to render active scene with error code: ", r);
      return r;
    }
    a->window.display();
  }

  for (auto &&cmd : a->commandQ)
    if (cmd)
      cmd(a);
  a->commandQ.clear();

  return fb::Result::Success;
}

template <typename T>
int ExtractParameterValue(int argc, char **argv, std::string regex, T *dst) {
  const std::regex query{regex};

  for (int i = 0; i < argc; ++i) {
    auto kvp = std::ranges::views::split(std::string{argv[i]}, '=');
    std::vector<std::string> seq{};
    for (auto it = kvp.begin(); it != kvp.end(); ++it)
      seq.push_back(std::string{(*it).begin(), (*it).end()});

    if (std::regex_match(seq.front(), query)) {
      std::string target{seq.back()};
      if (seq.size() == 1) {
        if (i + 1 == argc)
          return fb::Result::SyntaxError;
        target = argv[++i];
      }

      if constexpr (std::same_as<T, std::string>)
        *dst = target;
      else
        try {
          *dst = static_cast<T>(std::stold(target));
        } catch (...) {
          return fb::Result::ConversionError;
        }

      return fb::Result::Success;
    }
  }

  return fb::Result::NotFound;
}

fb::Button *CreateButton(auto &buttons, sf::Vector2f offset, sf::Vector2f sz,
                         fb::Button *prev = nullptr) {
  auto position =
      prev ? prev->box.getPosition() + prev->box.getSize() : sf::Vector2f{};
  buttons.push_back(fb::Button{});
  auto b = &buttons.back();

  b->box.setPosition(position + offset);
  b->box.setSize(sz);
  return b;
};

void UpdateButton(
    fb::Button *b, sf::Color initCol, sf::Color hoverCol, sf::Color clickCol,
    std::function<void(fb::Application *, fb::Button *)> cb = {}) {
  b->box.setFillColor(initCol);
  b->onUnHover = [initCol](fb::Application *, fb::Button *t) {
    t->box.setFillColor(initCol);
  };
  b->onHover = [hoverCol](fb::Application *, fb::Button *t) {
    t->box.setFillColor(hoverCol);
  };
  b->onUnClick = [initCol, cb](fb::Application *a, fb::Button *t) {
    t->box.setFillColor(initCol);
    if (cb)
      cb(a, t);
  };
  b->onClick = [clickCol](fb::Application *, fb::Button *t) {
    t->box.setFillColor(clickCol);
  };
};

void UpdateButtonText(auto &fonts, fb::Button *b, std::string f, sf::Color tc,
                      unsigned cs, std::string l) {
  b->text = std::unique_ptr<sf::Text>{new sf::Text{fonts.at(f)}};
  b->text->setCharacterSize(cs);
  b->text->setFillColor(tc);
  b->text->setString(l);
  b->centerText();
};

} // namespace

namespace fb {
int MainMenu::render() {
  for (auto &&b : buttons_)
    Render(app_, &b);
  return Result::Success;
}

int MainMenu::update() {
  for (auto it = buttons_.end(); true;) {
    --it;
    if (it != buttons_.end())
      it->update(app_);
    if (it == buttons_.begin())
      break;
  }
  return Result::Success;
}

int MainMenu::clear() {
  buttons_.clear();
  fonts_.clear();
  return Result::Success;
}

int MainMenu::build() {
  if (auto r = ReadFont(&fonts_, "ExoRegular", "./font/ExoRegular.ttf");
      r != Result::Success) {
    LogErr("Failed to read font: ExoRegular");
    return r;
  }

  const float vshift = (50.f / 540.f) * GetWindowSizeY(app_);
  const sf::Vector2f sz = {GetWindowSizeX(app_) * 0.4f, 150.f};
  const sf::Vector2f pos = {GetWindowSizeX(app_) / 2.f - sz.x / 2.f, vshift};

  const sf::Color ic(204, 51, 153), hc(230, 76, 178), cc(153, 0, 102);
  const unsigned cs = 100;
  std::string mainF = "ExoRegular";

  auto play = CreateButton(buttons_, pos, sz);
  UpdateButton(play, ic, hc, cc,
               [](auto *a, auto *) { ScheduleSceneTransition(a, "InGame"); });
  UpdateButtonText(fonts_, play, mainF, sf::Color::Black, cs, "Play");

  auto exit = CreateButton(buttons_, {-sz.x, vshift}, sz, play);
  UpdateButton(exit, ic, hc, cc, [](auto *a, auto *) { ScheduleExit(a); });
  UpdateButtonText(fonts_, exit, mainF, sf::Color::Black, cs, "Exit");

  return Result::Success;
}
} // namespace fb

namespace fb {
int InGame::render() {
  Render(app_, &bg_);
  for (auto &&f : fences_)
    Render(app_, &f);
  Render(app_, bird_.body.get());
  for (auto &&b : buttons_)
    Render(app_, &b);
  return Result::Success;
}

int InGame::update() {
  for (auto it = buttons_.end(); true;) {
    --it;
    if (it != buttons_.end())
      it->update(app_);
    if (it == buttons_.begin())
      break;
  }

  bird_.update(app_, !gameOver_);

  if (IsFlapRequested(app_))
    launch_ = true;

  if (launch_ && !gameOver_) {
    auto b = bird_.body.get();
    if (b) {
      v_ += dv_ * GetFrameTimeInSeconds(app_);
      b->move({0, v_});
    }

    if (IsFlapRequested(app_)) {
      v_ -= 1;
    }

    for (auto &&f : fences_) {
      if (auto bb = f.body.getGlobalBounds();
          bb.findIntersection(bird_.body->getGlobalBounds())) {
        gameOver_ = true;
        break;
      }

      f.update(app_, bird_.body->getGlobalBounds().size.x);

      if (bird_.body->getPosition().x >
          f.body.getPosition().x +
              3.f / 2.f * bird_.body->getGlobalBounds().size.x)
        if (f.score_) {
          IncrementScore(app_);
          f.score_ = false;
        }
    }

    const auto bb = bird_.body->getGlobalBounds();
    const auto p = bird_.body->getPosition();

    if (p.y < 0 || p.y > GetWindowSizeY(app_) - bb.size.y)
      gameOver_ = true;
  }

  return Result::Success;
}

int InGame::clear() {
  buttons_.clear();
  textures_.clear();
  fonts_.clear();
  fences_.clear();
  score_ = nullptr;
  scoreCount_ = 0;
  bird_ = {};
  launch_ = false;
  gameOver_ = false;
  v_ = 0;
  return Result::Success;
}
} // namespace fb

namespace {
int CreateInGameUI(fb::Application *app_, auto &fonts_, auto &buttons_,
                   auto &score_, auto &bg_) {
  if (auto r = fb::ReadFont(&fonts_, "ExoRegular", "./font/ExoRegular.ttf");
      r != fb::Result::Success) {
    fb::LogErr("Failed to read font: ExoRegular");
    return r;
  }

  const sf::Vector2f sz = {150.f, 50.f};
  const sf::Vector2f pos = {0, GetWindowSizeY(app_) - sz.y};

  const sf::Color ic(204, 51, 153), hc(230, 76, 178), cc(153, 0, 102);
  const unsigned cs = 50;
  std::string mainF = "ExoRegular";

  auto back = CreateButton(buttons_, pos, sz);
  UpdateButton(back, ic, hc, cc, [](auto *a, auto *) {
    ScheduleSceneClear(a);
    ScheduleSceneTransition(a, "MainMenu");
  });
  UpdateButtonText(fonts_, back, mainF, sf::Color::Black, cs, "Back");

  score_ = CreateButton(buttons_, {50.f, -sz.y}, {400.f, sz.y}, back);
  score_->box.setFillColor(ic);
  UpdateButtonText(fonts_, score_, mainF, sf::Color::Black, cs, "Score: 0");

  bg_.setSize({fb::GetWindowSizeX(app_), fb::GetWindowSizeY(app_)});
  bg_.setFillColor(sf::Color{75, 0, 130, 255});
  bg_.setPosition({});
  return fb::Result::Success;
}

int CreateInGameBird(fb::Application *app_, auto &textures_, auto &bird) {
  if (auto r =
          fb::ReadTexture(&textures_, "BirdSprite", "./img/BirdSprite.png");
      r != fb::Result::Success) {
    fb::LogErr("Failed to read bird sprite texture!");
    return r;
  }

  fb::Animation *animation{nullptr};
  fb::CreateAnimation(animation);
  bird.animation = std::unique_ptr<fb::Animation, int (*)(fb::Animation *)>{
      animation, fb::DestroyAnimation};

  fb::Frame *f1, *f2, *f3, *f4, *f5, *f6, *f7, *f8;
  fb::AddFrame(animation, f1, 0, 200, 160, 120);
  fb::AddFrame(animation, f2, 160, 200, 160, 120);
  fb::AddFrame(animation, f3, 320, 210, 160, 80);
  fb::AddFrame(animation, f4, 480, 190, 160, 100);
  fb::AddFrame(animation, f5, 640, 170, 160, 120);
  fb::AddFrame(animation, f6, 800, 180, 160, 100);
  fb::AddFrame(animation, f7, 960, 200, 160, 80);
  fb::AddFrame(animation, f8, 1120, 200, 160, 110);

  fb::FrameSeq *fly;
  fb::AddFrameSequence(animation, fly);
  fb::AddFrameToSequence(fly, f1);
  fb::AddFrameToSequence(fly, f2);
  fb::AddFrameToSequence(fly, f3);
  fb::AddFrameToSequence(fly, f4);
  fb::AddFrameToSequence(fly, f5);
  fb::AddFrameToSequence(fly, f6);
  fb::AddFrameToSequence(fly, f7);
  fb::AddFrameToSequence(fly, f8);

  fb::SetFrameSequence(animation, fly);
  fb::SetFrameDuration(fly, std::chrono::milliseconds{100});

  bird.body =
      std::unique_ptr<sf::Sprite>{new sf::Sprite{textures_.at("BirdSprite")}};
  bird.body->setTextureRect({{0, 200}, {160, 120}});
  if (fb::GetWindowSizeX(app_) < 1920)
    bird.body->scale({-0.5f, 0.5f});
  else
    bird.body->scale({-1.f, 1.f});
  const auto bb = bird.body->getGlobalBounds();
  bird.body->setPosition({(fb::GetWindowSizeX(app_) + bb.size.x) / 2.f,
                          fb::GetWindowSizeY(app_) / 2.f - bb.size.y});

  return fb::Result::Success;
}

int CreateInGameFences(fb::Application *app_, auto &fences_) {
  const auto wh = fb::GetWindowSizeY(app_);
  for (std::size_t i = 0; i < 2; ++i) {
    fences_.push_back({});
    auto b = &fences_.back().body;
    b->setSize(
        {50, wh - fb::GetRandomNumber(app_, 160 * 1.5f, 2.f / 3.f * wh)});
    b->setFillColor(sf::Color::Magenta);
    b->setPosition(
        {fb::GetWindowSizeX(app_) + i * (fb::GetWindowSizeX(app_) / 2.f),
         fb::GetRandomNumber(app_, 0, 2)
             ? 0
             : fb::GetWindowSizeY(app_) - b->getSize().y});
    fences_.back().up_ = (i % 2);
  }

  return fb::Result::Success;
}
} // namespace

namespace fb {
void Bird::update(Application *, bool animate) {
  if (animation && animate) {
    Frame *af{nullptr};
    GetActiveFrame(animation.get(), af);
    sf::IntRect r{};
    if (af) {
      GetFrameLeft(af, &r.position.x);
      GetFrameTop(af, &r.position.y);
      GetFrameWidth(af, &r.size.x);
      GetFrameHeight(af, &r.size.y);
      body->setTextureRect(r);
    }
  }
}

void Fence::update(Application *app, unsigned maxSpeed) {
  if (body.getPosition().x > -body.getSize().x) {
    body.move({-v_, 0});
    if (GetScore(app) >= 5) {
      body.move({0, up_ ? -2.f : 2.f});
      if (body.getPosition().y <= 0 && up_)
        up_ = false;
      if (body.getPosition().y >= GetWindowSizeY(app) - body.getSize().y &&
          !up_)
        up_ = true;
    }
  } else {
    const float wh = fb::GetWindowSizeY(app);
    body.setSize({50, (200.f / 540.f) * wh});
    body.setPosition({GetWindowSizeX(app), (float)fb::GetRandomNumber(
                                               app, 0, wh - body.getSize().y)});
    score_ = true;
  }

  if (v_ < maxSpeed)
    v_ += GetFrameTimeInSeconds(app) * 0.1;
}

int InGame::build() {
  if (auto r = CreateInGameUI(app_, fonts_, buttons_, score_, bg_);
      r != Result::Success) {
    LogErr("Failed to create InGame UI with error code: ", r);
    return r;
  }

  if (auto r = CreateInGameBird(app_, textures_, bird_); r != Result::Success) {
    LogErr("Failed to create bird with error code: ", r);
    return r;
  }

  if (auto r = CreateInGameFences(app_, fences_); r != Result::Success) {
    LogErr("Failed to create fences with error code: ", r);
    return r;
  }
  return Result::Success;
}
} // namespace fb
