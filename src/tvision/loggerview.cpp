#define Uses_TFrame
#define Uses_TDeskTop
#define Uses_TProgram
#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"

#include <tvision/tv.h>

#include "loggerview.h"

#pragma clang diagnostic pop

void LoggerViewer::print(const std::string &line) {
  if (this->out) {
    this->lock();
    *this->out << line;
    *this->out << flush;
    this->unlock();
  }
}

LoggerViewer::LoggerViewer(TRect bounds, const char *aTitle, ushort aBufSize, std::function<void()> onDestructFun)
    : TWindowInit(&LoggerViewer::initFrame),
      TWindow(bounds, aTitle, wnNoNumber),
      interior(0),
      out(0),
      onDestructFun(onDestructFun) {
  bounds = getExtent();
  bounds.grow(-1, -1);

  interior = new TTerminal(bounds, 0, standardScrollBar(sbVertical | sbHandleKeyboard), aBufSize);
  out = new otstream(interior);
  insert(interior);

  options |= ofTileable;
}

LoggerViewer::~LoggerViewer() {
  onDestructFun();
  delete out;
  title = 0;  // Not heap-allocated.
  out = 0;
}
