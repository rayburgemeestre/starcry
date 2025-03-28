#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TFileDialog
#define Uses_TKeys
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbitwise-instead-of-logical"

#include <tvision/tv.h>

#include "fileview.h"
#include "gadgets.h"
#include "loggerview.h"
#include "tstarcry.h"

#include <cmath>
#include <sstream>

#include "starcry/metrics.h"

#pragma clang diagnostic pop

const int hcFDosShell = 16, hcFExit = 17, hcFOFileOpenDBox = 31, hcFOpen = 14, hcFile = 13, hcWCascade = 22,
          hcWClose = 25, hcWNext = 23, hcWSizeMove = 19, hcWTile = 21, hcWZoom = 20, hcWindows = 18;

const int cmOpenCmd = 105, cmMetaViewCmd = 112, cmStdoutViewCmd = 113, cmFFmpegViewCmd = 114, cmTogglePreview = 115,
          cmMemoryViewCmd = 116;

TStarcry::TStarcry(metrics *metrics, std::function<void()> toggle_preview_callback)
    : TProgInit(&TStarcry::initStatusLine, &TStarcry::initMenuBar, &TStarcry::initDeskTop),
      metrics_(metrics),
      toggle_preview_callback_(toggle_preview_callback) {
  TRect r = getExtent();  // Create the clock view.
  r.a.x = r.b.x - 9;
  r.b.y = r.a.y + 1;
  clock = new TClockView(r);
  clock->growMode = gfGrowLoX | gfGrowHiX;
  insert(clock);

  r = getExtent();  // Create the sc eta view.
  r.a.x = r.b.x - 43;
  r.b.x = r.a.x + 23;
  r.b.y = r.a.y + 1;

  eta = new TQuickInfoView(r);
  eta->growMode = gfGrowLoX | gfGrowHiX;
  insert(eta);

  r = getExtent();  // Create the heap view.
  r.a.x = r.b.x - 23;
  r.b.x = r.a.x + 13;
  r.b.y = r.a.y + 1;
  heap = new THeapView(r);
  heap->growMode = gfGrowLoX | gfGrowHiX;
  insert(heap);

  startFileViewer(&meta_viewer_win, ":meta");
  startFileViewer(&stdout_viewer_win, ":stdout");
  startLoggerViewer(&ffmpeg_viewer, "ffmpeg");
  startLoggerViewer(&memory_viewer, "memory");

  void *p1 = 0;
  message(this, evCommand, cmTile, p1);
}

TPalette &TStarcry::getPalette() const {
  static TPalette palette(cpAppBlackWhite, sizeof(cpAppBlackWhite) - 1);
  return palette;
}

void TStarcry::exit() {
  exited = true;
}

void TStarcry::getEvent(TEvent &event) {
  TApplication::getEvent(event);
}

TStatusLine *TStarcry::initStatusLine(TRect r) {
  r.a.y = r.b.y - 1;

  return (new TStatusLine(r,
                          *new TStatusDef(0, 50) + *new TStatusItem("~Alt-X~ Exit", kbAltX, cmQuit) +
                              *new TStatusItem(0, kbAltF3, cmClose) + *new TStatusItem(0, kbF10, cmMenu) +
                              *new TStatusItem(0, kbF5, cmZoom) + *new TStatusItem(0, kbCtrlF5, cmResize) +
                              *new TStatusDef(50, 0xffff) + *new TStatusItem("Howdy", kbF1, cmHelp)));
}

void TStarcry::openFile(const char *fileSpec) {
  TFileDialog *d = (TFileDialog *)validView(new TFileDialog(fileSpec, "Open a File", "~N~ame", fdOpenButton, 100));

  if (d != 0 && deskTop->execView(d) != cmCancel) {
    char fileName[MAXPATH];
    d->getFileName(fileName);
    d->helpCtx = hcFOFileOpenDBox;
    TView *w = validView(new TFileWindow(fileName, []() {}));
    if (w != 0) deskTop->insert(w);
  }
  destroy(d);
}

void TStarcry::handleEvent(TEvent &event) {
  TApplication::handleEvent(event);

  if (event.what == evCommand) {
    switch (event.message.command) {
      case cmMetaViewCmd:
        startFileViewer(&meta_viewer_win, ":meta");
        break;
      case cmStdoutViewCmd:
        startFileViewer(&stdout_viewer_win, ":stdout");
        break;
      case cmFFmpegViewCmd:
        startLoggerViewer(&ffmpeg_viewer, "ffmpeg");
        break;
      case cmMemoryViewCmd:
        startLoggerViewer(&memory_viewer, "memory");
        break;
      case cmTogglePreview:
        toggle_preview_callback_();
        break;
      case cmOpenCmd:  //  View a file
        openFile("*.js");
        break;
      default:  //  Unknown command
        return;
    }
    void *p1 = 0;
    message(this, evCommand, cmTile, p1);
    clearEvent(event);
  } else if (event.what == evNothing && event.message.command == cmQuit) {
    user_exited = true;
  }
}

void TStarcry::startFileViewer(TFileWindow **window, const std::string &filename) {
  if (!*window) {
    *window = new TFileWindow(filename.c_str(), [=]() {
      *window = nullptr;
    });
    TView *w = validView(*window);
    deskTop->insert(w);
  }
}

void TStarcry::startLoggerViewer(LoggerViewer **viewer, const std::string &title) {
  if (!*viewer) {
    TRect bounds = TProgram::deskTop->getExtent();
    *viewer = (LoggerViewer *)validView(new LoggerViewer(bounds, title.c_str(), 0x0F00, [=]() {
      *viewer = nullptr;
    }));
    deskTop->insert(*viewer);
  }
}

static Boolean isTileable(TView *p, void *) {
  return ((p->options & ofTileable) != 0);
}

void TStarcry::idle() {
  TProgram::idle();
  clock->update();
  heap->update();
  eta->update();

  if (deskTop->firstThat(isTileable, 0) != 0) {
    enableCommand(cmTile);
    enableCommand(cmCascade);
  } else {
    disableCommand(cmTile);
    disableCommand(cmCascade);
  }

  if (meta_viewer_win) {
    meta_viewer_win->viewer->fileLines->freeAll();
  }
  if (stdout_viewer_win) {
    stdout_viewer_win->viewer->fileLines->freeAll();
  }
  if (ffmpeg_viewer) {
  }

  metrics_->display(
      [&](const std::string &line) {
        if (meta_viewer_win) {
          meta_viewer_win->viewer->fileLines->insert(newStr(line.c_str()));
        }
      },
      [&](const std::string &line) {
        if (stdout_viewer_win) {
          stdout_viewer_win->viewer->fileLines->insert(newStr(line.c_str()));
        }
      },
      [&](int level, const std::string &line) {
        if (ffmpeg_viewer) {
          ffmpeg_viewer->print(line);
        }
      },
      [&](const std::string &line) {
        if (memory_viewer) {
          memory_viewer->print(line);
        }
      });

  if (meta_viewer_win) {
    meta_viewer_win->viewer->draw();
    meta_viewer_win->viewer->setLimit(100, meta_viewer_win->viewer->fileLines->getCount());
  }
  if (stdout_viewer_win) {
    stdout_viewer_win->viewer->draw();
    stdout_viewer_win->viewer->setLimit(100, stdout_viewer_win->viewer->fileLines->getCount());
    stdout_viewer_win->viewer->scrollTo(0, stdout_viewer_win->viewer->fileLines->getCount());
  }

  if (exited) {
    void *p1 = 0;
    message(this, evCommand, cmQuit, p1);
  }
}

void TStarcry::setScript(const std::string &script) {
  if (script_filename_view != nullptr) deskTop->remove(script_filename_view);
  TView *script_filename_view = validView(new TFileWindow(script.c_str(), []() {}));
  if (script_filename_view != nullptr) deskTop->insert(script_filename_view);
  void *p1 = 0;
  message(this, evCommand, cmTile, p1);
}

void TStarcry::setProgress(int current_frame, int last_frame) {
  double progress = double(current_frame) / double(last_frame);
  progress *= 100;
  int percentage = round(progress);
  std::stringstream ss;
  ss << percentage << "%";
  eta->setText(ss.str());
}

TMenuBar *TStarcry::initMenuBar(TRect r) {
  TSubMenu &sub2 = *new TSubMenu("~F~ile", 0, hcFile) + *new TMenuItem("~O~pen...", cmOpenCmd, kbF3, hcFOpen, "F3") +
                   newLine() + *new TMenuItem("~M~eta Viewer", cmMetaViewCmd, kbAlt0, hcNoContext, "Alt-0") +
                   *new TMenuItem("~S~tdout Viewer", cmStdoutViewCmd, kbAlt1, hcNoContext, "Alt-1") +
                   *new TMenuItem("~F~Fmpeg Viewer", cmFFmpegViewCmd, kbAlt2, hcNoContext, "Alt-2") + newLine() +
                   *new TMenuItem("~M~emory Viewer", cmMemoryViewCmd, kbAlt3, hcNoContext, "Alt-3") + newLine() +
                   *new TMenuItem("~D~OS Shell", cmDosShell, kbNoKey, hcFDosShell) +
                   *new TMenuItem("~T~oggle Preview window", cmTogglePreview, kbCtrlT, hcNoContext, "Ctrl+T") +
                   *new TMenuItem("E~x~it", cmQuit, kbAltX, hcFExit, "Alt-X");

  TSubMenu &sub3 =
      *new TSubMenu("~W~indows", 0, hcWindows) +
      *new TMenuItem("~R~esize/move", cmResize, kbCtrlF5, hcWSizeMove, "Ctrl-F5") +
      *new TMenuItem("~Z~oom", cmZoom, kbF5, hcWZoom, "F5") + *new TMenuItem("~N~ext", cmNext, kbF6, hcWNext, "F6") +
      *new TMenuItem("~C~lose", cmClose, kbAltF3, hcWClose, "Alt-F3") +
      *new TMenuItem("~T~ile", cmTile, kbNoKey, hcWTile) + *new TMenuItem("C~a~scade", cmCascade, kbNoKey, hcWCascade);

  r.b.y = r.a.y + 1;
  return (new TMenuBar(r, sub2 + sub3));
}
