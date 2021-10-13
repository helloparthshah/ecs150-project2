// #include "RVCOS.h"
#include "RVCOS.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile int global = 42;
volatile uint32_t controller_status = 0;
volatile uint32_t cartridge_status = 0;

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

void writec(char c, int pos) { VIDEO_MEMORY[pos] = c; }

void write(const TTextCharacter *c, int start) {
  for (int i = 0; i < strlen(c); i++) {
    writec(c[i], start + i);
  }
}

volatile int cursor = 0;
TStatus RVCWriteText(const TTextCharacter *buffer, TMemorySize writesize) {
  write(buffer, cursor);
  cursor += writesize;
  if ((cursor & 0x3F) == 0x3F) {
    cursor -= 0x3F;
    cursor += 0x40;
  }
  return RVCOS_STATUS_SUCCESS;
}

void writei(uint32_t c, int start) {
  char hex[32];
  sprintf(hex, "%p", &c);
  write(hex, start);
  // RVCWriteText(hex, len(hex));
}

uint32_t bitExtracted(uint32_t number, uint32_t k, uint32_t p) {
  return (((1 << k) - 1) & (number >> (p - 1)));
}

// Setting the global pointer
__attribute__((always_inline)) inline void set_gp(uint32_t addr) {
  asm volatile("lw gp, %0" : : "g"(addr));
  // asm volatile("mv %0, gp" : "=g"(addr));
}

TStatus RVCInitalize(uint32_t *gp) {
  int *m = malloc(10 * sizeof(int));
  for (int i = 0; i < 10; i++) {
    m[i] = i + 1;
  }
  writei(m[0], 3 * 0x40);
  return RVCOS_STATUS_SUCCESS;
}

typedef void (*TFunctionPointer)(void);

volatile int isInit = 0;

extern uint32_t _heap_base;

int main() {
  int a = 4;
  int b = 12;
  int last_global = 42;
  int x_pos = 12;
  while (1) {
    int c = a + b + global;
    if (global != last_global) {
      // RVCWriteText("Text", 4);
      // writei(cartridge_status, 0);
      writei(global, 0x40);
      writei(_heap_base, 2 * 0x40);

      if (cartridge_status) {
        if (cartridge_status & 0x1 && isInit == 0) {
          isInit = 1;
          // cartridge_status &= ~(1UL << 0);
          // cartridge_status &= ~(1UL << 1);
          // global = cartridge_status;
          writei(cartridge_status & 0xFFFFFFFC, 0);
          TFunctionPointer FunPtr =
              (TFunctionPointer)(cartridge_status & 0xFFFFFFFC);
          FunPtr();
          // set_gp(cartridge_status);
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
