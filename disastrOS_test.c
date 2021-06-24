#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

#include "disastrOS.h"
#include "disastrOS_mailbox.h"
#include "disastrOS_globals.h"


#define MAX_SENDERS 10
#define MAX_RECEIVERS 10
#define NUM_MESSAGES_SENDER 130
#define NUM_MESSAGES_RECEIVER 130


// we need this to handle the sleep state
void sleeperFunction(void* args){
  if(_PRINTFUL_)  
    printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void senderFunction(void* args){
  if(_PRINTFUL_)
    printf("[SENDER %d] in\n",disastrOS_getpid());

  int* _args = (int*) args;
  int mailbox_id = *_args;

  //open mailbox
  int fd=disastrOS_openResource(mailbox_id,MAILBOX_TYPE,0);
  if(fd < 0){
    printf("[SENDER %d] Error: Cannot open mailbox resource! Exiting...\n", disastrOS_getpid()); 
    disastrOS_exit(0);
  } 

  int messages_counter = 0;
  //send
  char* message = "hi I'm writing in the mailbox";
  for(int i=0; i<NUM_MESSAGES_SENDER; i++){
    int tmp = disastrOS_send(fd,message);
    while(tmp == DSOS_EMAILBOXFULL){
      tmp = disastrOS_send(fd,message);
    }
    messages_counter++;
  }

  //close mailbox
  fd=disastrOS_closeResource(fd);
  if(fd<0)
    printf("[SENDER %d] Error: Cannot close mailbox resource!\n",disastrOS_getpid());

  if(_PRINTFUL_)
    printf("[SENDER %d] terminating\n", disastrOS_getpid());
  disastrOS_exit(messages_counter);
}

void receiverFunction(void* args){
  if(_PRINTFUL_)
    printf("[RECEIVER %d] in\n",disastrOS_getpid());

  int* _args = (int*) args;
  int mailbox_id = *_args;

  //open mailbox
  int fd=disastrOS_openResource(mailbox_id,MAILBOX_TYPE,0);
  if(fd < 0){
    printf("[RECEIVER %d] Error: Cannot open mailbox resource! Exiting...\n", disastrOS_getpid()); 
    disastrOS_exit(0);
  } 

  int messages_counter = 0;
  //receive
  for(int i=0; i<NUM_MESSAGES_RECEIVER; i++){
    char buffer[MAX_MESSAGE_LENGTH] = "";
    int tmp = disastrOS_receive(fd,buffer,MAX_MESSAGE_LENGTH);
    while(tmp == DSOS_EMAILBOXEMPTY){
      tmp = disastrOS_receive(fd,buffer,MAX_MESSAGE_LENGTH);
    }
    if(_PRINTFUL_)
      printf("[RECEIVER %d] received message of %d bytes\n",disastrOS_getpid(),(int)strlen(buffer));
    messages_counter++;
  }

  //close mailbox
  fd=disastrOS_closeResource(fd);
  if(fd<0)
    printf("[RECEIVER %d] Error: Cannot close mailbox resource!\n",disastrOS_getpid());

  if(_PRINTFUL_)
    printf("[RECEIVER %d] terminating\n", disastrOS_getpid());
  disastrOS_exit(-messages_counter);
}

void childFunction(void* args){
  if(_PRINTFUL_)
    printf("[CHILD %d] in\n",disastrOS_getpid());

  int* _args = (int*) args;
  int mailbox_id = *_args;


  //create mailbox
  if(_PRINTFUL_)
    printf("[CHILD %d] Creating mailbox = %d\n",disastrOS_getpid(),mailbox_id);
  int fd=disastrOS_openResource(mailbox_id,MAILBOX_TYPE,DSOS_CREATE);
  if(fd<0){
    printf("[CHILD %d] FATAL: Cannot create mailbox! Exiting...\n",disastrOS_getpid());
    disastrOS_exit(0);
  }

  int alive_children=0;
  
  //spawn receiver children
  for (int i=0; i<MAX_RECEIVERS; ++i) {
    disastrOS_spawn(receiverFunction, &mailbox_id);
    alive_children++;
  }
  //spawn sender children
  for (int i=0; i<MAX_SENDERS; ++i) {
    disastrOS_spawn(senderFunction, &mailbox_id);
    alive_children++;
  }
  
 
  //count messages left in the mailbox
  int messages_counter = 0;

  if(_PRINTFUL_)
    disastrOS_printStatus();

  //wait for children to terminate and count messages
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    if(_PRINTFUL_)
      printf("[CHILD %d], child: %d terminated, retval:%d, alive: %d \n", disastrOS_getpid(),pid, retval, --alive_children);
    messages_counter += retval;
  }
  
  printf("\n\t[CHILD %d] (messages sent - messages received) = %d\n\n",disastrOS_getpid(),messages_counter);

  disastrOS_exit(messages_counter);
}


void initFunction(void* args) {
  if(_PRINTFUL_)
    disastrOS_printStatus();
  if(_PRINTFUL_)
    printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);

  int alive_children=0;
  int _args[MAX_NUM_MAILBOXES] = {0};
  for (int i=0; i < MAX_NUM_MAILBOXES; ++i) {
    _args[i] = i;
    disastrOS_spawn(childFunction, (void*) &_args[i]);
    alive_children++;
  }

  int total = 0;
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){
    if(_PRINTFUL_)
      printf("initFunction, child: %d terminated, retval:%d, alive: %d \n", pid, retval, alive_children);
    total += retval;
    --alive_children;
  }

  printf("\n\nTOTAL SCORE %d ->",total);
  if(total == 0) printf(" SUCCESS :)\n\n");
  else printf(" FAILURE :(\n\n");

  //destroy mailboxes
  for(int i=0; i<MAX_NUM_MAILBOXES; i++){
    if(disastrOS_destroyResource(i) < 0)
      printf("[INIT] Error: Cannot destroy mailbox %d!\n",i);
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
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
