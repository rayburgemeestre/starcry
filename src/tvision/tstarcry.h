#pragma once

#define Uses_TApplication
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#include <tvision/tv.h>
#pragma clang diagnostic pop

#include <atomic>
#include <memory>

class TStatusLine;
class TMenuBar;
struct TEvent;
class TQuickInfoView;
class THeapView;
class TClockView;
class LoggerViewer;
class TFileWindow;

// starcry
class metrics;

class TStarcry : public TApplication {
public:
  explicit TStarcry(metrics *metrics);
  static TStatusLine *initStatusLine(TRect r);
  static TMenuBar *initMenuBar(TRect r);
  virtual void handleEvent(TEvent &Event);
  virtual void getEvent(TEvent &event);
  virtual void idle();
  void exit();

private:
  TQuickInfoView *eta = nullptr;
  THeapView *heap = nullptr;
  TClockView *clock = nullptr;

  TFileWindow *meta_viewer_win = nullptr;
  TFileWindow *stdout_viewer_win = nullptr;
  LoggerViewer *ffmpeg_viewer = nullptr;

  void startFileViewer(TFileWindow **window, const std::string &filename);
  void startLoggerViewer(LoggerViewer **viewer, const std::string &title);

  void openFile(const char *fileSpec);
  metrics *metrics_;
  std::atomic<bool> exited = false;
};