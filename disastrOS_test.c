#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_mailbox.h"
#include "disastrOS_globals.h"

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void senderFunction(void* args){
  printf("[SENDER %d] in\n",disastrOS_getpid());

  //open mailbox
  int fd=disastrOS_openResource(MAILBOX_ID,1,0);
  if(fd < 0){
    printf("[SENDER %d] Error: Cannot open mailbox resource! Exiting...\n", disastrOS_getpid()); 
    disastrOS_exit(disastrOS_getpid()+1);
  } 

  //send
  char* message = "hi I'm writing in the mailbox";
  for(int i=0; i<130; i++){
    int tmp = disastrOS_send(MAILBOX_ID,message);
    while(tmp == DSOS_EMAILBOXFULL){
      tmp = disastrOS_send(MAILBOX_ID,message);
    }
  }

  //close mailbox
  fd=disastrOS_closeResource(fd);
  if(fd<0)
    printf("[SENDER %d] Error: Cannot close mailbox resource!\n",disastrOS_getpid());

  printf("[SENDER %d] terminating\n", disastrOS_getpid());
  disastrOS_exit(disastrOS_getpid()+1);
}

void receiverFunction(void* args){
  printf("[RECEIVER %d] in\n",disastrOS_getpid());

  //open mailbox
  int fd=disastrOS_openResource(MAILBOX_ID,1,0);
  if(fd < 0){
    printf("[RECEIVER %d] Error: Cannot open mailbox resource! Exiting...\n", disastrOS_getpid()); 
    disastrOS_exit(disastrOS_getpid()+1);
  } 

  //receive
  for(int i=0; i<130; i++){
    char* buffer = 0;
    int tmp = disastrOS_receive(MAILBOX_ID,&buffer);
    while(tmp == DSOS_EMAILBOXEMPTY){
      tmp = disastrOS_receive(MAILBOX_ID,&buffer);
    }
    printf("[RECEIVER %d] message received = %s\n",disastrOS_getpid(),buffer);
  }

  //close mailbox
  fd=disastrOS_closeResource(fd);
  if(fd<0)
    printf("[RECEIVER %d] Error: Cannot close mailbox resource!\n",disastrOS_getpid());

  printf("[RECEIVER %d] terminating\n", disastrOS_getpid());
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);

  int fd=disastrOS_openResource(MAILBOX_ID,1,DSOS_CREATE);
  if(fd<0){
    printf("[INIT] FATAL: Cannot create mailbox! Exiting...\n");
    disastrOS_shutdown();
  }

  int alive_children=0;
  
  //spawn receiver children
  for (int i=0; i<MAX_RECEIVERS; ++i) {
    disastrOS_spawn(receiverFunction, 0);
    alive_children++;
  }
  //spawn sender children
  for (int i=0; i<MAX_SENDERS; ++i) {
    disastrOS_spawn(senderFunction, 0);
    alive_children++;
  }
  
  
  disastrOS_printStatus();
  //wait for children to terminate
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, --alive_children);
  }
  printf("shutdown!\n");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  //printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
