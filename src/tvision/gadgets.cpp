#define Uses_TRect
#define Uses_TView
#define Uses_TDrawBuffer

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#include <tvision/tv.h>

#include "gadgets.h"
#pragma clang diagnostic pop

#include <malloc.h>
#include <string.h>
#include <time.h>
#include <iomanip>
#include <sstream>

TQuickInfoView::TQuickInfoView(TRect &r) : TView(r) {
  str = "TODO";
}

void TQuickInfoView::draw() {
  TDrawBuffer buf;
  TColorAttr c = getColor(2);

  buf.moveChar(0, ' ', c, (short)size.x);
  buf.moveStr(0, str.c_str(), c);
  writeLine(0, 0, (short)size.x, 1, buf);
}

void TQuickInfoView::update() {
  // TODO: only invoke drawView if string has changed
  drawView();
}

//
// ------------- Heap Viewer functions
//

THeapView::THeapView(TRect &r) : TView(r) {
  oldMem = 0;
  newMem = heapSize();
}

void THeapView::draw() {
  TDrawBuffer buf;
  TColorAttr c = getColor(2);

  buf.moveChar(0, ' ', c, (short)size.x);
  buf.moveStr(0, heapStr, c);
  writeLine(0, 0, (short)size.x, 1, buf);
}

void THeapView::update() {
  if ((newMem = heapSize()) != oldMem) {
    oldMem = newMem;
    drawView();
  }
}

long THeapView::heapSize() {
  std::stringstream totalStr;  //(heapStr, sizeof heapStr);

#if defined(__GLIBC__) && !defined(__UCLIBC__) && !defined(__MUSL__)
  // mallinfo is defined in malloc.h but only exists in Glibc.
  // It doesn't exactly measure the heap size, but it kinda does the trick.
  int allocatedBytes =
#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 33)
      mallinfo2()
#else
      mallinfo()
#endif
          .uordblks;
  totalStr << std::setw(12) << allocatedBytes;
  strncpy(heapStr, totalStr.str().c_str(), sizeof heapStr);
  return allocatedBytes;
#else
  return 0;
#endif
}

//
// -------------- Clock Viewer functions
//

TClockView::TClockView(TRect &r) : TView(r) {
  strcpy(lastTime, "        ");
  strcpy(curTime, "        ");
}

void TClockView::draw() {
  TDrawBuffer buf;
  TColorAttr c = getColor(2);

  buf.moveChar(0, ' ', c, (short)size.x);
  buf.moveStr(0, curTime, c);
  writeLine(0, 0, (short)size.x, 1, buf);
}

void TClockView::update() {
  time_t t = time(0);
  char *date = ctime(&t);

  date[19] = '\0';
  strcpy(curTime, &date[11]); /* Extract time. */

  if (strcmp(lastTime, curTime)) {
    drawView();
    strcpy(lastTime, curTime);
  }
}
