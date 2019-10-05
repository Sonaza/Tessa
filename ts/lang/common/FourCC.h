#pragma once

// Combine 4 bytes into a single 32-bit integer
#define TS_FOURCC(a, b, c, d) ( (((a) & 0xFF) << 24) | (((b) & 0xFF) << 16) | \
                                (((c) & 0xFF) << 8)  |  ((d) & 0xFF) )
