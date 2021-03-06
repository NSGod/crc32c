// Copyright 2008,2009,2010 Massachusetts Institute of Technology.
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#ifndef LOGGING_CRC32C_H__
#define LOGGING_CRC32C_H__

#if defined(__cplusplus)
#include <cstddef>
#else
#include <stdlib.h>
#endif

#include <stdint.h>
#include "crc32c_tables.h"

#if defined(__cplusplus)
namespace logging {
#endif

/** Returns the initial value for a CRC32-C computation. */
static inline uint32_t crc32cInit() {
    return 0xFFFFFFFF;
}

/** Pointer to a function that computes a CRC32C checksum.
@arg crc Previous CRC32C value, or crc32c_init().
@arg data Pointer to the data to be checksummed.
@arg length length of the data in bytes.
*/
typedef uint32_t (*CRC32CFunctionPtr)(uint32_t crc, const void* data, size_t length);

/** This will map automatically to the "best" CRC implementation. */
extern CRC32CFunctionPtr crc32c;

CRC32CFunctionPtr detectBestCRC32C(void);

/** Converts a partial CRC32-C computation to the final value. */
static inline uint32_t crc32cFinish(uint32_t crc) {
    return ~crc;
}

uint32_t crc32cSarwate(uint32_t crc, const void* data, size_t length);
uint32_t crc32cSlicingBy4(uint32_t crc, const void* data, size_t length);
uint32_t crc32cSlicingBy8(uint32_t crc, const void* data, size_t length);
    
#if !((defined __ppc__) || (defined __ppc64__))
uint32_t crc32cHardware32(uint32_t crc, const void* data, size_t length);
uint32_t crc32cHardware64(uint32_t crc, const void* data, size_t length);
#endif // !((defined __ppc__) || (defined __ppc64__))
    
#if defined(__cplusplus)
}  // namespace logging
#endif

#endif
