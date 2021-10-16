// #include "RVCOS.h"
#include "Deque.h"
#include "RVCOS.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile int global = 42;
volatile uint32_t controller_status = 0;
volatile uint32_t cartridge_status = 0;

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

void writec(TTextCharacter c, int pos) { VIDEO_MEMORY[pos] = c; }

void write(const TTextCharacter *c, int start) {
  for (int i = 0; i < strlen(c); i++) {
    writec(c[i], start + i);
  }
}

void writei(uint32_t c, int start) {
  char hex[32];
  sprintf(hex, "%x", c);
  write(hex, start);
}

volatile struct Deque *TCB;

volatile uint32_t cart_gp = 0;
TStatus RVCInitialize(uint32_t *gp) {
  TCB = dmalloc();
  cart_gp = (uint32_t)gp;
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCTickMS(uint32_t *tickmsref) { return RVCOS_STATUS_SUCCESS; }
TStatus RVCTickCount(TTickRef tickref) { return RVCOS_STATUS_SUCCESS; }

TStatus RVCThreadCreate(TThreadEntry entry, void *param, TMemorySize memsize,
                        TThreadPriority prio, TThreadIDRef tid) {
  struct Thread t;
  t.id = *tid;
  t.priority = prio;
  t.memSize = memsize;
  t.state = RVCOS_THREAD_STATE_CREATED;
  push_back(TCB, t);
  return RVCOS_STATUS_SUCCESS;
}
TStatus RVCThreadDelete(TThreadID thread) { return RVCOS_STATUS_SUCCESS; }
TStatus RVCThreadActivate(TThreadID thread) { return RVCOS_STATUS_SUCCESS; }
TStatus RVCThreadTerminate(TThreadID thread, TThreadReturn returnval) {
  return RVCOS_STATUS_SUCCESS;
}
TStatus RVCThreadWait(TThreadID thread, TThreadReturnRef returnref) {
  return RVCOS_STATUS_SUCCESS;
}
TStatus RVCThreadID(TThreadIDRef threadref) { return RVCOS_STATUS_SUCCESS; }
TStatus RVCThreadState(TThreadID thread, TThreadStateRef stateref) {
  return RVCOS_STATUS_SUCCESS;
}
TStatus RVCThreadSleep(TTick tick) { return RVCOS_STATUS_SUCCESS; }

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
TStatus RVCReadController(SControllerStatusRef statusref) {
  statusref->DLeft = controller_status & 0x1;
  statusref->DUp = controller_status & 0x2;
  statusref->DDown = controller_status & 0x3;
  statusref->DRight = controller_status & 0x4;
  statusref->DButton1 = controller_status & 0x5;
  statusref->DButton2 = controller_status & 0x6;
  statusref->DButton3 = controller_status & 0x7;
  statusref->DButton4 = controller_status & 0x8;
  statusref->DReserved = controller_status >> 8;
  return RVCOS_STATUS_SUCCESS;
}

void enter_cartridge();

volatile int isInit = 0;

extern uint32_t _heap_base;

volatile uint32_t *saved_sp;

int main() {
  saved_sp = &controller_status;

  while (1) {
    writei(global, 9 * 0x40);
    if (cartridge_status & 0x1 && isInit == 0) {
      isInit = 1;
      write("Cartridge entered", 8 * 0x40);
      enter_cartridge();
    }
    if (!(cartridge_status & 0x1) && isInit == 1) {
      isInit = 0;
      write("                 ", 8 * 0x40);
    }
  }
  return 0;
}

uint32_t c_syscall_handler(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3,
                           uint32_t a4, uint32_t code) {
  writei(code, 0);
  TStatus status = RVCOS_STATUS_FAILURE;
  if (code == 0) {
    status = RVCInitialize(a0);
  } else if (code == 1) {
    status = RVCThreadCreate(a0, a1, a2, a3, a4);
  } else if (code == 2) {
    status = RVCThreadDelete(a0);
  } else if (code == 3) {
    status = RVCThreadActivate(a0);
  } else if (code == 4) {
    status = RVCThreadTerminate(a0, a1);
  } else if (code == 5) {
    status = RVCThreadWait(a0, a1);
  } else if (code == 6) {
    status = RVCThreadID(a0);
  } else if (code == 7) {
    status = RVCThreadState(a0, a1);
  } else if (code == 8) {
    status = RVCThreadSleep(a0);
  } else if (code == 9) {
    status = RVCTickMS(a0);
  } else if (code == 10) {
    status = RVCTickCount(a0);
  } else if (code == 11) {
    status = RVCWriteText(a0, a1);
  } else if (code == 12) {
    status = RVCReadController(a0);
  }
  return status;
}
