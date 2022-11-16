#ifndef TESTLIB_H
#define TESTLIB_H

#include "Arduino.h"

class testLib {

public:
  void begin(int interruptPin);
  void classInterruptHandler(void);
  void setCallback(void (*userDefinedCallback)(const int)) {
                      localPointerToCallback = userDefinedCallback; }

private:
  int localInterruptPin; // need to store pin because it will be used in another function
  void (*localPointerToCallback)(const int);

} ;

#endif