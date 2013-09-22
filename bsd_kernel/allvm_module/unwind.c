// Stubs to make linker happy.

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h> /* uprintf */

#define WARN() uprintf("Unsupported call to '%s'!\n", __FUNCTION__)

#define STUB(name) \
  void name(void); \
  void name() { WARN(); }

STUB(_Unwind_DeleteException)
STUB(_Unwind_GetDataRelBase)
STUB(_Unwind_GetIPInfo)
STUB(_Unwind_GetLanguageSpecificData)
STUB(_Unwind_GetRegionStart)
STUB(_Unwind_GetTextRelBase)
STUB(_Unwind_RaiseException)
STUB(_Unwind_Resume)
STUB(_Unwind_Resume_or_Rethrow)
STUB(_Unwind_SetGR)
STUB(_Unwind_SetIP)

