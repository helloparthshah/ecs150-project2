// #include "RVCOS.h"
#include "Deque.h"
#include "RVCOS.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile int global = 42;
#define CONTROLLER (*((volatile uint32_t *)0x40000018))

volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);
#define CARTRIDGE (*((volatile uint32_t *)0x4000001C))

/* __attribute__((always_inline)) inline void set_tp(uint32_t tp) {
  asm volatile("sa tp, %0" ::"r"(tp));
} */

__attribute__((always_inline)) inline uint32_t get_tp(void) {
  uint32_t result;
  asm volatile("la tp, %0" ::"r"(result));
  return result;
}

void write(const TTextCharacter *c, uint32_t start) {
  for (uint32_t i = 0; i < strlen(c); i++) {
    VIDEO_MEMORY[start + i] = c[i];
  }
}

void writei(uint32_t c, int line) {
  char hex[32];
  sprintf(hex, "%x", c);
  write(hex, line * 0x40);
}

void idleThread() {
  while (1)
    ;
}

volatile Deque *sched;
volatile Thread tcb[256];
volatile int cid = 0;

volatile uint32_t cart_gp;
TStatus RVCInitialize(uint32_t *gp) {
  sched = dmalloc();
  cart_gp = gp;
  // Create idle thread
  tcb[cid].sp = malloc(1024);
  tcb[cid].entry = idleThread;
  tcb[cid].id = cid;
  tcb[cid].priority = RVCOS_THREAD_PRIORITY_LOW;
  tcb[cid].memSize = 1024;
  tcb[cid].state = RVCOS_THREAD_STATE_CREATED;
  cid++;
  // Create main thread
  tcb[cid].sp = malloc(1024);
  tcb[cid].entry = idleThread;
  tcb[cid].id = cid;
  tcb[cid].priority = RVCOS_THREAD_PRIORITY_LOW;
  tcb[cid].memSize = 1024;
  tcb[cid].state = RVCOS_THREAD_STATE_CREATED;
  cid++;
  // Make tp point to main
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCTickMS(uint32_t *tickmsref) { return RVCOS_STATUS_SUCCESS; }

TStatus RVCTickCount(TTickRef tickref) { return RVCOS_STATUS_SUCCESS; }

TStatus RVCThreadCreate(TThreadEntry entry, void *param, TMemorySize memsize,
                        TThreadPriority prio, TThreadIDRef tid) {
  tcb[cid].entry = entry;
  tcb[cid].id = cid;
  *tid = cid;
  tcb[cid].priority = prio;
  tcb[cid].memSize = memsize;
  tcb[cid].sp = malloc(memsize);
  tcb[cid].state = RVCOS_THREAD_STATE_CREATED;
  push_back((Deque *)sched, tcb[cid++]);
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

TStatus RVCThreadID(TThreadIDRef threadref) {
  *threadref = get_tp();
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadState(TThreadID thread, TThreadStateRef stateref) {
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadSleep(TTick tick) { return RVCOS_STATUS_SUCCESS; }

volatile int cursor = 0;
TStatus RVCWriteText(const TTextCharacter *buffer, TMemorySize writesize) {
  for (uint32_t i = 0; i < writesize; i++) {
    if (buffer[i] == '\b') {
      if (cursor > 0)
        VIDEO_MEMORY[--cursor] = ' ';
    } else if (buffer[i] == '\n') {
      cursor += 0x40;
      cursor -= cursor % 0x40;
    } else {
      VIDEO_MEMORY[cursor++] = buffer[i];
    }
    if ((cursor) / 0x40 >= 36) {
      memcpy(VIDEO_MEMORY, VIDEO_MEMORY + 0x40, 0x40 * 36);
      cursor -= 0x40;
    }
  }
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCReadController(SControllerStatusRef statusref) {
  if (!statusref)
    return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
  statusref->DLeft = CONTROLLER & 0x1 ? 1 : 0;
  statusref->DUp = CONTROLLER & 0x2 ? 1 : 0;
  statusref->DDown = CONTROLLER & 0x4 ? 1 : 0;
  statusref->DRight = CONTROLLER & 0x8 ? 1 : 0;
  statusref->DButton1 = CONTROLLER & 0x10 ? 1 : 0;
  statusref->DButton2 = CONTROLLER & 0x20 ? 1 : 0;
  statusref->DButton3 = CONTROLLER & 0x40 ? 1 : 0;
  statusref->DButton4 = CONTROLLER & 0x80 ? 1 : 0;
  statusref->DReserved = CONTROLLER >> 0x8;
  return RVCOS_STATUS_SUCCESS;
}

void enter_cartridge();

volatile int isInit = 0;

volatile uint32_t *saved_sp;

int main() {
  saved_sp = &CONTROLLER;
  char i = 'a';
  while (1) {
    // writei(global, 9);
    // writei(CONTROLLER, 8);
    if (CARTRIDGE & 0x1 && isInit == 0) {
      isInit = 1;
      enter_cartridge();
    }
    if (!(CARTRIDGE & 0x1) && isInit == 1) {
      isInit = 0;
    }
  }
  return 0;
}

uint32_t c_syscall_handler(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3,
                           uint32_t a4, uint32_t code) {
  if (code == 0) {
    return RVCInitialize(a0);
  } else if (code == 1) {
    return RVCThreadCreate(a0, a1, a2, a3, a4);
  } else if (code == 2) {
    return RVCThreadDelete(a0);
  } else if (code == 3) {
    return RVCThreadActivate(a0);
  } else if (code == 4) {
    return RVCThreadTerminate(a0, a1);
  } else if (code == 5) {
    return RVCThreadWait(a0, a1);
  } else if (code == 6) {
    return RVCThreadID(a0);
  } else if (code == 7) {
    return RVCThreadState(a0, a1);
  } else if (code == 8) {
    return RVCThreadSleep(a0);
  } else if (code == 9) {
    return RVCTickMS(a0);
  } else if (code == 10) {
    return RVCTickCount(a0);
  } else if (code == 11) {
    return RVCWriteText(a0, a1);
  } else if (code == 12) {
    return RVCReadController(a0);
  }
  return RVCOS_STATUS_FAILURE;
}
