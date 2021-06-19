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
  Mailbox* mailbox = MailboxList_byId(&mailboxes_list,MAILBOX_ID);
  if(mailbox == NULL){
    printf("[SENDER %d] Error: Cannot find mailbox! Exiting...\n", disastrOS_getpid());
    disastrOS_exit(disastrOS_getpid()+1);
  }
  int fd=disastrOS_openResource(mailbox->resource->id,0,0);
  if(fd < 0){
    printf("[SENDER %d] Error: Cannot open mailbox resource! Exiting...\n", disastrOS_getpid()); 
    disastrOS_exit(disastrOS_getpid()+1);
  } 
  //printf("[SENDER %d] fd=%d\n", disastrOS_getpid(), fd);

  //send
  char* message = "hi I'm writing in the mailbox";
  if(disastrOS_send(MAILBOX_ID,message) < 0)
    printf("[SENDER %d] ERROR: Cannot send message!\n",disastrOS_getpid());
  
  //sleep
  disastrOS_sleep(5);

  /*fd=disastrOS_closeResource(mailbox->resource->id);
  if(fd<0)
    printf("[SENDER %d] Error: Cannot close mailbox resource!\n",disastrOS_getpid());*/

  printf("[SENDER %d] terminating\n", disastrOS_getpid());
  disastrOS_exit(disastrOS_getpid()+1);
}

void receiverFunction(void* args){
  printf("[RECEIVER %d] in\n",disastrOS_getpid());

  //open mailbox
  Mailbox* mailbox = MailboxList_byId(&mailboxes_list,MAILBOX_ID);
  if(mailbox == NULL){
    printf("[RECEIVER %d] Error: Cannot find mailbox! Exiting...\n", disastrOS_getpid());
    disastrOS_exit(disastrOS_getpid()+1);
  }
  int fd=disastrOS_openResource(mailbox->resource->id,0,0);
  if(fd < 0){
    printf("[RECEIVER %d] Error: Cannot open mailbox resource! Exiting...\n", disastrOS_getpid()); 
    disastrOS_exit(disastrOS_getpid()+1);
  } 
  //printf("[RECEIVER %d] fd=%d\n", disastrOS_getpid(), fd);

  //receive
  if(disastrOS_receive(MAILBOX_ID) < 0)
    printf("[RECEIVER %d] ERROR: Cannot send message!\n",disastrOS_getpid());
  
  //sleep
  disastrOS_sleep(5);

  /*fd=disastrOS_closeResource(mailbox->resource->id);
  if(fd<0)
    printf("[RECEIVER %d] Error: Cannot close mailbox resource!\n",disastrOS_getpid());*/

  printf("[RECEIVER %d] terminating\n", disastrOS_getpid());
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);

  //create mailbox
  Mailbox* mailbox = Mailbox_alloc(MAILBOX_ID,0);
  if(mailbox == NULL){
    printf("[INIT] FATAL: Cannot create mailbox! Exiting...\n");
    disastrOS_shutdown();
  }
  List_insert(&mailboxes_list, mailboxes_list.last, (ListItem*) mailbox);

  int alive_children=0;
  
  //spawn sender children
  for (int i=0; i<MAX_SENDERS; ++i) {
    disastrOS_spawn(senderFunction, 0);
    alive_children++;
  }
  //spawn receiver children
  for (int i=0; i<MAX_RECEIVERS; ++i) {
    disastrOS_spawn(receiverFunction, 0);
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
