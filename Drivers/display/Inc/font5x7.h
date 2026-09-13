#ifndef FONT5X7_H
#define FONT5X7_H
#include <stdint.h>
/* Shared font: five visible bits per row (bit 4 is leftmost), seven rows.
 * Lowercase maps to uppercase; unsupported characters render as '?'. */
uint8_t Font5x7_Row(unsigned char c, unsigned row);
#endif
