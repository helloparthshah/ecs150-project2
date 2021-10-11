// #include "RVCOS.h"
#include "RVCOS.h"
#include <stdint.h>
#include <stdlib.h>

volatile int global = 42;
volatile uint32_t controller_status = 0;
volatile uint32_t cartridge_status = 0;

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

void *_sbrk(incr) int incr;
{
  extern char _heapbase; /* Set by linker.  */
  static char *heap_end;
  char *prev_heap_end;

  if (heap_end == 0)
    heap_end = &_heapbase;

  prev_heap_end = heap_end;
  heap_end += incr;

  return (void *)prev_heap_end;
}

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

void writei(uint32_t c, int start, int size) {
  for (int i = 0; i < size; i++) {
    writec(((c >> i) & 1) + '0', start + size - 1 - i);
  }
}

uint32_t bitExtracted(uint32_t number, uint32_t k, uint32_t p) {
  return (((1 << k) - 1) & (number >> (p - 1)));
}

TStatus RVCInitalize(uint32_t *gp) {
  int *m = (int *)malloc(10 * sizeof(uint32_t));
  return RVCOS_STATUS_SUCCESS;
}

volatile int isInit = 0;

int main() {
  int a = 4;
  int b = 12;
  int last_global = 42;
  int x_pos = 12;

  while (1) {
    int c = a + b + global;
    if (global != last_global) {
      writei(global, 0x40, 32);
      writei(bitExtracted(cartridge_status, 29, 2), 0, 30);
      if (cartridge_status) {
        if (cartridge_status & 0x1 && isInit == 0) {
          isInit = 1;
          // itoa((cartridge_status >> 0) & 1, buf, 10);
          RVCInitalize((uint32_t *)&cartridge_status);
          global = cartridge_status >> 2;
        }
      }

      if (controller_status) {
        writec(' ', x_pos);
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
