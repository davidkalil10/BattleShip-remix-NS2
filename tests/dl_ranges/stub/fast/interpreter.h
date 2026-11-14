#pragma once

/* Test stand-in for libultraship's fast/interpreter.h: only what port_dl_ranges.cpp registers with. */

#include <cstddef>
#include <cstdint>

namespace Fast {

typedef int (*DLBoundsCheckFn)(uintptr_t addr);
typedef int (*DLRangeLookupFn)(uintptr_t addr, uintptr_t* rangeBase, size_t* rangeSize);
typedef int (*AddressClassifierFn)(uintptr_t addr, char* buf, size_t bufSize);

inline void RegisterDLBoundsCheck(DLBoundsCheckFn) {}
inline void RegisterDLRangeLookup(DLRangeLookupFn) {}
inline void RegisterAddressClassifier(AddressClassifierFn) {}

} // namespace Fast
