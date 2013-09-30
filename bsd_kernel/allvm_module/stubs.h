#ifndef _STUBS_H_
#define _STUBS_H_

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h> /* uprintf */

//#define BAIL() panic("Unsupported call to '%s'!\n", __FUNCTION__)
#define BAIL() printf("Unsupported call to '%s'!\n", __FUNCTION__)

#define UNSUPPORTED(name) \
  void name(void); \
  void name() { BAIL(); }

#define TRACE() printf("[TRACE] Call to '%s'\n", __FUNCTION__)

#endif // _STUBS_H_
