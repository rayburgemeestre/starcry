#pragma once

#define Uses_TEvent
#define Uses_TRect
#define Uses_TView
#include <tvision/tv.h>

class TQuickInfoView : public TView {
public:
  TQuickInfoView(TRect& r);
  virtual void update();
  virtual void draw();

private:
  std::string str;
};

class THeapView : public TView {
public:
  THeapView(TRect& r);
  virtual void update();
  virtual void draw();
  virtual long heapSize();

private:
  long oldMem, newMem;
  char heapStr[16];
};

class TClockView : public TView {
public:
  TClockView(TRect& r);
  virtual void draw();
  virtual void update();

private:
  char lastTime[9];
  char curTime[9];
};