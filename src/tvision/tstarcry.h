#pragma once

#define Uses_TApplication
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wbitwise-instead-of-logical"
#else
#endif
#include <tvision/tv.h>
#ifdef __clang__
#pragma clang diagnostic pop
#pragma clang diagnostic pop
#else
#endif

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
  explicit TStarcry(metrics *metrics, std::function<void()> toggle_preview_callback);
  static TStatusLine *initStatusLine(TRect r);
  static TMenuBar *initMenuBar(TRect r);
  virtual void handleEvent(TEvent &Event);
  virtual void getEvent(TEvent &event);
  virtual void idle();
  virtual TPalette &getPalette() const;
  void setScript(const std::string &script);
  void setProgress(int current_frame, int last_frame);
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

  TView *script_filename_view = nullptr;

  std::function<void()> toggle_preview_callback_;

public:
  std::atomic<bool> exited = false;
  std::atomic<bool> user_exited = false;
};