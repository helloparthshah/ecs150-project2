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

void write(const TTextCharacter *c, uint32_t line) {
  for (uint32_t i = 0; i < strlen(c); i++) {
    VIDEO_MEMORY[line * 0x40 + i] = c[i];
  }
}

void writei(uint32_t c, int line) {
  char hex[32];
  sprintf(hex, "%x", c);
  write(hex, line);
}

typedef uint32_t (*TEntry)(uint32_t param);

__attribute__((always_inline)) inline void set_tp(uint32_t tp) {
  asm volatile("lw tp, 0(%0)" ::"r"(&tp));
}

__attribute__((always_inline)) inline uint32_t get_tp(void) {
  uint32_t result;
  asm volatile("mv %0,tp" : "=r"(result));
  return result;
}

// uint32_t get_tp();

__attribute__((always_inline)) inline void set_ra(uint32_t tp) {
  asm volatile("lw ra, 0(%0)" ::"r"(tp));
}

extern void csr_write_mie(uint32_t);

extern void csr_enable_interrupts(void);

extern void csr_disable_interrupts(void);

uint32_t idleThread(uint32_t param) {
  write("Idle", 63);
  csr_enable_interrupts();
  while (1)
    ;
}

void switch_context(uint32_t **oldctx, uint32_t *newctx);

uint32_t call_on_other_gp(uint32_t param, TEntry entry, uint32_t gp);

uint32_t *initialize_stack(uint32_t *sp, TEntry fun, uint32_t param,
                           uint32_t tp) {
  sp--;
  *sp = fun; // sw      ra,48(sp)
  sp--;
  *sp = tp; // sw      tp,44(sp)
  sp--;
  *sp = 0; // sw      t0,40(sp)
  sp--;
  *sp = 0; // sw      t1,36(sp)
  sp--;
  *sp = 0; // sw      t2,32(sp)
  sp--;
  *sp = 0; // sw      s0,28(sp)
  sp--;
  *sp = 0; // sw      s1,24(sp)
  sp--;
  *sp = param; // sw      a0,20(sp)
  sp--;
  *sp = 0; // sw      a1,16(sp)
  sp--;
  *sp = 0; // sw      a2,12(sp)
  sp--;
  *sp = 0; // sw      a3,8(sp)
  sp--;
  *sp = 0; // sw      a4,4(sp)
  sp--;
  *sp = 0; // sw      a5,0(sp)
  return sp;
}

volatile Deque *high;
volatile Deque *norm;
volatile Deque *low;
volatile Thread tcb[256];
volatile int id_count = 0;
volatile int curr_running = 1;
volatile int ticks = 0;
volatile uint32_t cart_gp;

void scheduler() {
  uint32_t old_running = curr_running;

  if (tcb[old_running].state == RVCOS_THREAD_STATE_RUNNING) {
    tcb[old_running].state = RVCOS_THREAD_STATE_READY;

    if (tcb[old_running].priority == RVCOS_THREAD_PRIORITY_LOW)
      push_back((Deque *)low, old_running);
    else if (tcb[old_running].priority == RVCOS_THREAD_PRIORITY_NORMAL)
      push_back((Deque *)norm, old_running);
    else if (tcb[old_running].priority == RVCOS_THREAD_PRIORITY_HIGH)
      push_back((Deque *)high, old_running);
  }

  // print(high, 8);
  if (high->head)
    writei(high->head->val, 8);

  if (isEmpty(high) == 0) {
    curr_running = pop_front(high);
  } else if (isEmpty(norm) == 0) {
    curr_running = pop_front(norm);
  } else if (isEmpty(low) == 0) {
    curr_running = pop_front(low);
  } else {
    curr_running = 0;
  }

  if (curr_running == 2)
    write("Running 2", 9);

  // if (tcb[curr_running].state == RVCOS_THREAD_STATE_READY) {
  tcb[curr_running].state = RVCOS_THREAD_STATE_RUNNING;

  if (old_running != curr_running)
    switch_context((uint32_t **)&tcb[old_running].ctx, tcb[curr_running].ctx);
  // }
}

void skeleton() {
  set_tp(curr_running);
  uint32_t (*entry)(void *) = tcb[curr_running].entry;
  csr_enable_interrupts();
  uint32_t ret_value = call_on_other_gp(tcb[curr_running].param,
                                        tcb[curr_running].entry, cart_gp);
  csr_disable_interrupts();
  RVCThreadTerminate(curr_running, ret_value);
}

TStatus RVCInitialize(uint32_t *gp) {
  high = dmalloc();
  norm = dmalloc();
  low = dmalloc();
  cart_gp = gp;
  // Create idle thread
  tcb[id_count].ctx = malloc(1024);
  tcb[id_count].entry = idleThread;
  tcb[id_count].id = id_count;
  tcb[id_count].priority = 0;
  tcb[id_count].memsize = 1024;
  tcb[id_count].state = RVCOS_THREAD_STATE_CREATED;
  id_count++;
  // Create main thread
  tcb[id_count].ctx = NULL;
  tcb[id_count].entry = NULL;
  tcb[id_count].id = id_count;
  tcb[id_count].priority = RVCOS_THREAD_PRIORITY_NORMAL;
  tcb[id_count].memsize = 0;
  tcb[id_count].state = RVCOS_THREAD_STATE_RUNNING;
  // Make tp point to main
  set_tp(tcb[id_count++].id);
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCTickMS(uint32_t *tickmsref) {
  if (!tickmsref)
    return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
  *tickmsref = 2;
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCTickCount(TTickRef tickref) {
  *tickref = ticks;
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadCreate(TThreadEntry entry, void *param, TMemorySize memsize,
                        TThreadPriority prio, TThreadIDRef tid) {
  tcb[id_count].entry = entry;
  tcb[id_count].id = id_count;
  *tid = id_count;
  tcb[id_count].priority = prio;
  tcb[id_count].param = param;
  tcb[id_count].memsize = memsize;
  tcb[id_count].state = RVCOS_THREAD_STATE_CREATED;
  id_count++;
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadDelete(TThreadID thread) {
  Thread t;
  free(tcb[thread].ctx);
  tcb[thread] = t;
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadActivate(TThreadID thread) {
  // print(low, 15);

  tcb[thread].ctx = initialize_stack(malloc(tcb[thread].memsize), skeleton,
                                     tcb[thread].param, id_count);
  tcb[thread].state = RVCOS_THREAD_STATE_READY;
  if (tcb[thread].priority == RVCOS_THREAD_PRIORITY_LOW)
    push_back((Deque *)low, thread);
  else if (tcb[thread].priority == RVCOS_THREAD_PRIORITY_NORMAL)
    push_back((Deque *)norm, thread);
  else if (tcb[thread].priority == RVCOS_THREAD_PRIORITY_HIGH)
    push_back((Deque *)high, thread);

  print(high, 16);

  scheduler();
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadTerminate(TThreadID thread, TThreadReturn returnval) {
  tcb[thread].state = RVCOS_THREAD_STATE_DEAD;
  tcb[thread].return_val = returnval;
  // If current is waited by then set all to ready
  if (tcb[thread].waited_by != NULL)
    while (isEmpty(tcb[thread].waited_by) == 0) {
      uint32_t wid = pop_front(tcb[thread].waited_by);
      tcb[wid].state = RVCOS_THREAD_STATE_READY;
      if (tcb[wid].priority == RVCOS_THREAD_PRIORITY_LOW)
        push_back((Deque *)low, wid);
      else if (tcb[wid].priority == RVCOS_THREAD_PRIORITY_NORMAL)
        push_back((Deque *)norm, wid);
      else if (tcb[wid].priority == RVCOS_THREAD_PRIORITY_HIGH)
        push_back((Deque *)high, wid);
    }

  if (tcb[thread].priority == RVCOS_THREAD_PRIORITY_LOW)
    removeT((Deque *)low, thread);
  else if (tcb[thread].priority == RVCOS_THREAD_PRIORITY_NORMAL)
    removeT((Deque *)norm, thread);
  else if (tcb[thread].priority == RVCOS_THREAD_PRIORITY_HIGH)
    removeT((Deque *)high, thread);

  scheduler();
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadWait(TThreadID thread, TThreadReturnRef returnref) {
  TThreadID wid = curr_running;
  tcb[wid].state = RVCOS_THREAD_STATE_WAITING;

  if (tcb[wid].priority == RVCOS_THREAD_PRIORITY_LOW)
    removeT((Deque *)low, wid);
  else if (tcb[wid].priority == RVCOS_THREAD_PRIORITY_NORMAL)
    removeT((Deque *)norm, wid);
  else if (tcb[wid].priority == RVCOS_THREAD_PRIORITY_HIGH)
    removeT((Deque *)high, wid);

  if (tcb[thread].waited_by == NULL)
    tcb[thread].waited_by = dmalloc();
  push_back(tcb[thread].waited_by, wid);

  while (tcb[thread].state != RVCOS_THREAD_STATE_DEAD) {
    scheduler();
  }

  *returnref = tcb[thread].return_val;
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadID(TThreadIDRef threadref) {
  *threadref = get_tp();
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadState(TThreadID thread, TThreadStateRef stateref) {
  *stateref = tcb[thread].state;
  return RVCOS_STATUS_SUCCESS;
}

TStatus RVCThreadSleep(TTick tick) {
  scheduler();
  return RVCOS_STATUS_SUCCESS;
}

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
  csr_enable_interrupts();
  csr_write_mie(0x888);
  saved_sp = &CONTROLLER;
  while (1) {
    writei(global, 20);
    writei(CARTRIDGE & 0x1, 21);
    if (CARTRIDGE & 0x1 && isInit == 0) {
      isInit = 1;
      csr_disable_interrupts();
      enter_cartridge();
    }
    if (!(CARTRIDGE & 0x1) && isInit == 1) {
      csr_enable_interrupts();
      cursor = 0;
      for (int i = 0; i < 36 * 64; i++) {
        VIDEO_MEMORY[i] = ' ';
      }
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
