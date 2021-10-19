#pragma once

#define Uses_TCollection
#define Uses_TScroller
#define Uses_TWindow
#include <tvision/tv.h>

#include <functional>

class TLineCollection : public TCollection {
public:
  TLineCollection(short lim, short delta) : TCollection(lim, delta) {}
  virtual void freeItem(void *p) {
    delete[](char *) p;
  }

private:
  virtual void *readItem(ipstream &) {
    return 0;
  }
  virtual void writeItem(void *, opstream &) {}
};

class TFileViewer : public TScroller {
public:
  char *fileName;
  TCollection *fileLines;
  Boolean isValid;
  TFileViewer(const TRect &bounds, TScrollBar *aHScrollBar, TScrollBar *aVScrollBar, const char *aFileName);
  ~TFileViewer();
  TFileViewer(StreamableInit) : TScroller(streamableInit){};
  void draw();
  void readFile(const char *fName);
  void setState(ushort aState, Boolean enable);
  void scrollDraw();
  Boolean valid(ushort command);

private:
  virtual const char *streamableName() const {
    return name;
  }

protected:
  virtual void write(opstream &);
  virtual void *read(ipstream &);

public:
  static const char *const name;
  static TStreamable *build();
};

class TFileWindow : public TWindow {
public:
  TFileWindow(const char *fileName, std::function<void()> onDestroyFun);
  ~TFileWindow();

  TFileViewer *viewer = nullptr;
  std::function<void()> onDestroyFun;
};

const int maxLineLength = 256;