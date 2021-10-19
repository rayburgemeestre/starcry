#pragma once

#define Uses_TWindow
#define Uses_TRect
#define Uses_TTerminal
#define Uses_otstream
#include <tvision/tv.h>
#include <functional>

class LoggerViewer : public TWindow {
public:
  LoggerViewer(TRect bounds, const char *aTitle, ushort aBufSize, std::function<void()> onDestructFun);
  ~LoggerViewer();

  void print(const std::string &line);

private:
  TTerminal *interior;
  otstream *out;
  std::function<void()> onDestructFun;
};
