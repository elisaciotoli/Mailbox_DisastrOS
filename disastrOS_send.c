#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_resource.h"
#include "disastrOS_mailbox.h"
#include "disastrOS_descriptor.h"

void internal_send() {
  int mailbox_fd = running->syscall_args[0];
  char* text = (char*)running->syscall_args[1];

  //find the mailbox
  Descriptor* des=DescriptorList_byFd(&running->descriptors, mailbox_fd);
  if (! des){
    running->syscall_retvalue=DSOS_ERESOURCECLOSE;
    return;
  }
  Mailbox* mailbox = (Mailbox*) ResourceList_byId(&resources_list,des->resource->id);
  if(! mailbox){
    printf("[SEND %d] Error: Cannot find mailbox!\n",disastrOS_getpid());
    return;
  }


  //wait if necessary
  if((&mailbox->messages_list)->size == MAX_NUM_MESSAGES_PER_MAILBOX){

    if(_PRINTFUL_)
      printf("[SEND %d] in wait \n",disastrOS_getpid());

    //put running in waiting list
    running->status=Waiting;
    running->syscall_retvalue=DSOS_EMAILBOXFULL;
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);

    //put running pointer in the mailbox waiting list
    PCBPtr* running_ptr = PCBPtr_alloc(running);
    if(List_insert(&(mailbox->waiting_list),(mailbox->waiting_list).last, (ListItem*) running_ptr) == 0)
      printf("[SEND %d] Error: Cannot write in mailbox waiting list\n",disastrOS_getpid());

    // pick the next running
    PCB* next_running= (PCB*) List_detach(&ready_list, ready_list.first);
    running=next_running;
    return;
  }


  //write message in queue
  if(_PRINTFUL_)
    printf("[SEND %d] inserting message\n",disastrOS_getpid());
    
  running->syscall_retvalue=0;
  Message* mes = Message_alloc(text);
  List_insert(&mailbox->messages_list,mailbox->messages_list.last,(ListItem*) mes);

  if(_PRINTFUL_)
    printf("[SEND %d] message inserted of %d bytes\n",disastrOS_getpid(),mes->size);


  //if necessary unblock processes waiting to read
  if((&mailbox->waiting_list)->size > 0){
    
    if(_PRINTFUL_)
      printf("[SEND %d] putting blocked processes in ready\n",disastrOS_getpid());

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
      printf("[SEND %d] all blocked processes in ready\n",disastrOS_getpid());
  }

  if(_PRINTFUL_)
    printf("[SEND %d] exiting\n",disastrOS_getpid());

  return;

}
