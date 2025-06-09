#pragma once

namespace sf {
class Drawable;
}

namespace fb {
/* This structure controls the basic aspects of the program.
 * For instance, the max framerate, the window properties,
 * etc...
 */
struct Application;

int Initialize(Application *&, int, char **);
int Run(Application *);
void Destroy(Application *);
void ScheduleExit(Application *);
void ScheduleSceneTransition(Application *, const char *);
void ScheduleSceneClear(Application *);

/* Returns the time spent on processing the most recent completed frame */
double GetFrameTimeInSeconds(Application *);

float GetMousePositionX(Application *);
float GetMousePositionY(Application *);
float GetWindowSizeX(Application *);
float GetWindowSizeY(Application *);
unsigned GetScore(Application *);
void IncrementScore(Application *);

bool IsPrimaryMouseButtonPressed(Application *);
void SetButtonClicked(Application *, bool);
bool IsButtonClicked(Application *);
void SetButtonHovered(Application *, bool);
bool IsButtonHovered(Application *);
bool IsFlapRequested(Application *);

unsigned GetRandomNumber(Application *, unsigned inclBegin, unsigned exclEnd);

void Render(Application *, sf::Drawable *);

void LogErr(const char *, int);
void LogErr(const char *);
} // namespace fb
