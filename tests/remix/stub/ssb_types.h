#ifndef TESTS_REMIX_STUB_SSB_TYPES_H
#define TESTS_REMIX_STUB_SSB_TYPES_H

/* Test stand-in for decomp/include/ssb_types.h. The rule files are plain C and use none of its types: some include
 * it before <math.h> only so the Switch build gets its wint_t typedef. The real header needs decomp/include on the
 * include path, whose libc shims (stddef.h, stdlib.h, string.h) break a host build. */

#endif
