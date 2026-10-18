#ifndef AE3_NATIVE_DISPATCH_H
#define AE3_NATIVE_DISPATCH_H
#include "ae3_scalar_leaves.h"
/* Unified native code dispatch for recovered routines; DOES NOT implement PS2 hardware. */
int ae3_run_native_leaf(AE3_ScalarContext *ctx, uint32_t pc);
size_t ae3_native_leaf_count(void);
uint32_t ae3_native_leaf_address(size_t index);
#endif
