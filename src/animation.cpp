#include <animation.hpp>
#include <chrono>
#include <list>
#include <result.hpp>

namespace fb {
struct Frame {
  int left, top, width, height;
};

struct Animation {
  std::chrono::time_point<std::chrono::steady_clock> stamp;
  std::list<Frame> frames;
  std::list<FrameSeq> sequences;
  FrameSeq *activeSeq;
};

struct FrameSeq {
  std::chrono::milliseconds duration;
  std::list<Frame *> frames;
  Frame *activeFrame;
};

int AddFrame(Animation *a, Frame *&f, int left, int top, int width,
             int height) {
  a->frames.push_back({left, top, width, height});
  f = &a->frames.back();
  return Result::Success;
}

int CreateAnimation(Animation *&a) {
  a = new Animation{};
  if (!a)
    return Result::Error;
  return Result::Success;
}

int DestroyAnimation(Animation *a) {
  delete a;
  return Result::Success;
}

int AddFrameSequence(Animation *a, FrameSeq *&f) {
  a->sequences.push_back({});
  f = &a->sequences.back();
  return Result::Success;
}

int AddFrameToSequence(FrameSeq *s, Frame *f) {
  if (!s->frames.size())
    s->activeFrame = f;
  s->frames.push_back(f);
  return Result::Success;
}

int SetFrameDuration(FrameSeq *s, std::chrono::milliseconds d) {
  s->duration = d;
  return Result::Success;
}

int SetFrameSequence(Animation *a, FrameSeq *s) {
  a->activeSeq = s;
  return Result::Success;
}

int GetFrameDuration(FrameSeq *s, std::chrono::milliseconds *dst) {
  *dst = s->duration;
  return Result::Success;
}

int GetActiveFrame(Animation *a, Frame *&f) {
  if (!a->activeSeq)
    return Result::DomainError;

  const auto now = std::chrono::steady_clock::now();
  using ms = std::chrono::milliseconds;
  ms duration{};

  GetFrameDuration(a->activeSeq, &duration);

  if (std::chrono::duration_cast<ms>(now - a->stamp) > duration) {
    a->stamp = now;
    auto &frames = a->activeSeq->frames;
    auto it = frames.begin();
    for (; it != frames.end(); ++it)
      if (*it == a->activeSeq->activeFrame)
        break;
    ++it;
    if (it == frames.end())
      a->activeSeq->activeFrame = frames.front();
    else
      a->activeSeq->activeFrame = *it;
  }

  f = a->activeSeq->activeFrame;
  return Result::Success;
}

int GetFrameLeft(Frame *f, int *v) {
  *v = f->left;
  return Result::Success;
}

int GetFrameTop(Frame *f, int *v) {
  *v = f->top;
  return Result::Success;
}

int GetFrameWidth(Frame *f, int *v) {
  *v = f->width;
  return Result::Success;
}

int GetFrameHeight(Frame *f, int *v) {
  *v = f->height;
  return Result::Success;
}
} // namespace fb
