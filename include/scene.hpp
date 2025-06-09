#pragma once

namespace fb {
struct Application;

struct Scene {
protected:
  Application *app_{nullptr};
  bool rebuild_{false};

public:
  Scene(Application *ptr) : app_{ptr} {}
  virtual ~Scene() = default;
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;
  Scene(Scene &&) = default;
  Scene &operator=(Scene &&) = default;

  virtual int build() = 0;
  virtual int update() = 0;
  virtual int render() = 0;
  virtual int clear() = 0;

  bool requiresRebuild() const { return rebuild_; }
  void requiresRebuild(bool v) { rebuild_ = v; }
};
} // namespace fb
