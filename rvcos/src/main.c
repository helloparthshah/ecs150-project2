// #include "RVCOS.h"
#include <stdint.h>

volatile int global = 42;
volatile uint32_t controller_status = 0;
volatile uint32_t cartridge_status = 0;

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

int len(char *p) {
  int c = 0;
  while (*p != '\0') {
    c++;
    *p++;
  }
  return (c);
}

void writec(char c, int pos) { VIDEO_MEMORY[pos] = c; }

void write(char *c, int start) {
  for (int i = start; i < start + len(c); i++) {
    writec(c[i], i);
  }
}

int main() {
  int a = 4;
  int b = 12;
  int last_global = 42;
  int x_pos = 12;

  write("Hello World!X", 0);

  while (1) {
    int c = a + b + global;
    if (global != last_global) {
      if (cartridge_status)
        writec(cartridge_status + '0', 0);

      if (controller_status) {
        writec(controller_status + '0', 1);
        VIDEO_MEMORY[x_pos] = ' ';
        VIDEO_MEMORY[0] = ' ';
        if (controller_status & 0x1) {
          if (x_pos & 0x3F) {
            x_pos--;
          }
        }
        if (controller_status & 0x2) {
          if (x_pos >= 0x40) {
            x_pos -= 0x40;
          }
        }
        if (controller_status & 0x4) {
          if (x_pos < 0x8C0) {
            x_pos += 0x40;
          }
        }
        if (controller_status & 0x8) {
          if ((x_pos & 0x3F) != 0x3F) {
            x_pos++;
          }
        }
        writec('X', x_pos);
        // VIDEO_MEMORY[x_pos] = 'X';
      }
      last_global = global;
    }
  }
  return 0;
}
