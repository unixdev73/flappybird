#pragma once

#include <chrono>

namespace fb {
struct Animation;
struct Frame;
struct FrameSeq;

int CreateAnimation(Animation *&);
int DestroyAnimation(Animation *);
int AddFrame(Animation *, Frame *&, int left, int top, int width, int height);
int AddFrameSequence(Animation *, FrameSeq *&);
int AddFrameToSequence(FrameSeq *, Frame *);

int SetFrameDuration(FrameSeq *, std::chrono::milliseconds);
int GetFrameDuration(FrameSeq *, std::chrono::milliseconds *);
int SetFrameSequence(Animation *, FrameSeq *);
int GetActiveFrame(Animation *, Frame *&);

int GetFrameLeft(Frame *, int *);
int GetFrameTop(Frame *, int *);
int GetFrameWidth(Frame *, int *);
int GetFrameHeight(Frame *, int *);
} // namespace fb
