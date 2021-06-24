#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_globals.h"
#include "disastrOS_mailbox.h"
#include "disastrOS_descriptor.h"

void internal_receive() {
  int mailbox_fd=running->syscall_args[0];
  char* buffer=(char*) running->syscall_args[1];
  
  //find the mailbox
  Descriptor* des=DescriptorList_byFd(&running->descriptors, mailbox_fd);
  if (! des){
    running->syscall_retvalue=DSOS_ERESOURCECLOSE;
    return;
  }
  Mailbox* mailbox = (Mailbox*) ResourceList_byId(&resources_list,des->resource->id);
  if(! mailbox){
    printf("[RECEIVE %d] Error: Cannot find mailbox!\n",disastrOS_getpid());
    return;
  }
  
  
  //wait if necessary
  if((&mailbox->messages_list)->size == 0){

    if(_PRINTFUL_)
      printf("[RECEIVE %d] in wait \n",disastrOS_getpid());

    //put running in waiting list
    running->status=Waiting;
    running->syscall_retvalue=DSOS_EMAILBOXEMPTY;
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);

    //put running pointer in the mailbox waiting list
    PCBPtr* running_ptr = PCBPtr_alloc(running);
    if(List_insert(&mailbox->waiting_list,mailbox->waiting_list.last, (ListItem*) running_ptr) == 0)
      printf("[RECEIVE %d] Error: Cannot write in mailbox waiting list\n",disastrOS_getpid());
    
    // pick the next running
    PCB* next_running= (PCB*) List_detach(&ready_list, ready_list.first);
    next_running->status=Running;
    running=next_running;
    return;
  }


  //read message from queue
  if(_PRINTFUL_)
    printf("[RECEIVE %d] detaching message\n",disastrOS_getpid());

  running->syscall_retvalue=0;
  Message* message = (Message*) List_detach(&mailbox->messages_list,mailbox->messages_list.first);
  
  //copy message in buffer
  int cnt = 0;
  while(cnt < MAX_MESSAGE_LENGTH && message->text[cnt] != '\0'){
    buffer[cnt] = message->text[cnt];
    cnt++;
  }
  if(cnt < MAX_MESSAGE_LENGTH)
    buffer[cnt] = '\0';

  if(Message_free(message)<0) 
    printf("[RECEIVE %d] Errore: Cannot free message!\n",disastrOS_getpid());


  //if necessary unblock processes waiting to write
  if((&mailbox->waiting_list)->size > 0){

    if(_PRINTFUL_)
      printf("[RECEIVE %d] putting blocked processes in ready\n",disastrOS_getpid());

    ListItem* aux=List_detach(&mailbox->waiting_list,mailbox->waiting_list.first);
    while(aux){
      PCBPtr* pcb_aux = (PCBPtr*)aux;
      PCB* pcb_to_wake=(PCB*)pcb_aux->pcb;
      PCBPtr_free(pcb_aux);
      List_detach(&waiting_list, (ListItem*) pcb_to_wake);
      pcb_to_wake->status=Ready;
      List_insert(&ready_list, ready_list.last, (ListItem*) pcb_to_wake);
      if((&mailbox->waiting_list)->size > 0)
        aux=List_detach(&mailbox->waiting_list,mailbox->waiting_list.first);
      else 
        aux = NULL;
    }
    
    if(_PRINTFUL_)
      printf("[RECEIVE %d] all blocked processes in ready\n",disastrOS_getpid());
  }
  
  if(_PRINTFUL_)
    printf("[RECEIVE %d] exiting\n",disastrOS_getpid());
  
  return;
  
}
