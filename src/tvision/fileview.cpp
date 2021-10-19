#define Uses_MsgBox
#define Uses_TKeys
#define Uses_TScroller
#define Uses_TDrawBuffer
#define Uses_TRect
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TStreamableClass
#define Uses_ipstream
#define Uses_opstream

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"

#include <tvision/tv.h>

#include "fileview.h"

#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-volatile"
__link(RScroller) __link(RScrollBar)
#pragma clang diagnostic pop

#include <cstdlib>
#include <cstring>

#include <fstream>
#include <sstream>

    const char *const TFileViewer::name = "TFileViewer";

TFileViewer::TFileViewer(const TRect &bounds, TScrollBar *aHScrollBar, TScrollBar *aVScrollBar, const char *aFileName)
    : TScroller(bounds, aHScrollBar, aVScrollBar) {
  growMode = gfGrowHiX | gfGrowHiY;
  isValid = True;
  fileName = 0;
  readFile(aFileName);
}

TFileViewer::~TFileViewer() {
  delete[] fileName;
  destroy(fileLines);
}

void TFileViewer::draw() {
  char *p;

  TColorAttr c = getColor(1);
  for (short i = 0; i < size.y; i++) {
    TDrawBuffer b;
    b.moveChar(0, ' ', c, (short)size.x);

    if (delta.y + i < fileLines->getCount()) {
      p = (char *)(fileLines->at(delta.y + i));
      if (p) b.moveStr(0, p, c, (short)size.x, (short)delta.x);
    }
    writeBuf(0, i, (short)size.x, 1, b);
  }
}

void TFileViewer::scrollDraw() {
  TScroller::scrollDraw();
  draw();
}

void TFileViewer::readFile(const char *fName) {
  delete[] fileName;

  limit.x = 0;
  fileName = newStr(fName);
  fileLines = new TLineCollection(5, 5);
  std::ifstream fileToView(fName);
  if (std::string(fName).at(0) != ':' && !fileToView) {
    std::stringstream os;
    os << "Failed to open file '" << fName << "'.";
    messageBox(os.str().c_str(), mfError | mfOKButton);
    isValid = False;
  } else {
    char *line = (char *)malloc(maxLineLength);
    size_t lineSize = maxLineLength;
    char c;
    while (!lowMemory() && !fileToView.eof() && fileToView.get(c)) {
      size_t i = 0;
      while (!fileToView.eof() && c != '\n' && c != '\r')  // read a whole line
      {
        if (i == lineSize) line = (char *)realloc(line, (lineSize *= 2));
        line[i++] = c ? c : ' ';
        fileToView.get(c);
      }
      line[i] = '\0';
      if (c == '\r' && fileToView.peek() == '\n') fileToView.get(c);  // grab trailing newline on CRLF
      limit.x = max(limit.x, strwidth(line));
      fileLines->insert(newStr(line));
    }
    isValid = True;
    ::free(line);
  }
  limit.y = fileLines->getCount();
}

void TFileViewer::setState(ushort aState, Boolean enable) {
  TScroller::setState(aState, enable);
  if (enable && (aState & sfExposed)) setLimit(limit.x, limit.y);
}

Boolean TFileViewer::valid(ushort) {
  return isValid;
}

void *TFileViewer::read(ipstream &is) {
  char *fName;

  TScroller::read(is);
  fName = is.readString();
  fileName = 0;
  readFile(fName);
  delete fName;
  return this;
}

void TFileViewer::write(opstream &os) {
  TScroller::write(os);
  os.writeString(fileName);
}

TStreamable *TFileViewer::build() {
  return new TFileViewer(streamableInit);
}

TStreamableClass RFileView(TFileViewer::name, TFileViewer::build, __DELTA(TFileViewer));

static short winNumber = 0;

TFileWindow::TFileWindow(const char *fileName, std::function<void()> onDestroyFun)
    : TWindowInit(&TFileWindow::initFrame),
      TWindow(TProgram::deskTop->getExtent(), fileName, winNumber++),
      onDestroyFun(onDestroyFun) {
  options |= ofTileable;
  TRect r(getExtent());
  r.grow(-1, -1);
  viewer = new TFileViewer(r,
                           standardScrollBar(sbHorizontal | sbHandleKeyboard),
                           standardScrollBar(sbVertical | sbHandleKeyboard),
                           fileName);
  insert(viewer);
}

TFileWindow::~TFileWindow() {
  onDestroyFun();
}