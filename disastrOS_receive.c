#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"

void internal_receive() {
  int id=running->syscall_args[0];
  
  printf("[RECEIVE] ok\n");
  return;
  
}
