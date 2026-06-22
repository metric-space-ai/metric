// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_MACRODISABLE_H
#define METRIC_NUMERIC_SYSTEM_MACRODISABLE_H
// These macros are defined in several Linux headers (asm/termbits.h, asm-generic/termbits.h
// and bits/termios.h) and not properly undefined.
#undef VT1
#undef VT2

#endif
