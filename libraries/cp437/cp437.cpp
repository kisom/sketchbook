#include <SPI.h>
#include <GD.h>

#include "cp437_data.h"
#include "cp437.h"

int screen_addr(int x, int y)
{
  return (y << 7) + x;
}

void drawstr(uint16_t addr, const char *s)
{
  while (*s) {
    uint16_t w = pgm_read_word(cp437_pic + 2 * *s);
    GD.wr(addr, lowByte(w));
    GD.wr(addr + 64, highByte(w));
    s++, addr++;
  }
}

void cp437_setup()
{
	GD.uncompress(RAM_CHR, (prog_uchar *)cp437_chr);
	GD.uncompress(RAM_PAL, (prog_uchar *)cp437_pal);
}
