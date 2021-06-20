#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_globals.h"
#include "disastrOS_mailbox.h"

void internal_receive() {
  int id=running->syscall_args[0];
  
  Mailbox* mailbox = (Mailbox*) ResourceList_byId(&resources_list,id);
  if(mailbox == NULL){
    printf("[RECEIVE] Error: Cannot find mailbox!\n");
    return;
  }
  
  //wait
  if((&mailbox->messages_list)->size == 0){
    //put running in waiting list
    printf("[RECEIVE %d] in wait \n",disastrOS_getpid());
    running->status=Waiting;
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);

    //put running pointer in the mailbox waiting list
    PCBPtr* running_ptr = PCBPtr_alloc(running);
    if(List_insert(&mailbox->waiting_list,mailbox->waiting_list.last, (ListItem*) running_ptr) == 0)
      printf("[RECEIVE] Error: Cannot write in mailbox waiting list\n");
    printf("\n[RECEIVE] Mailbox Waiting List: ");
    PCBPtrList_print(&mailbox->waiting_list);
    // pick the next running
    PCB* next_running= (PCB*) List_detach(&ready_list, ready_list.first);
    next_running->status=Running;
    running=next_running;
    //disastrOS_printStatus();
  }
  else{

    printf("[RECEIVE] detaching message\n");
    
    Message* message = (Message*) List_detach(&mailbox->messages_list,mailbox->messages_list.first);
    char* text = message->text;
    printf("[RECEIVE] message received = %s\n",text);
    if(Message_free(message)<0)
      printf("[RECEIVE] Errore: Cannot free message!\n");

    if((&mailbox->waiting_list)->size > 0){
      printf("[RECEIVE] putting blocked processes in ready\n");
      ListItem* aux=List_detach(&mailbox->waiting_list,mailbox->waiting_list.first);
      while(aux){
        PCBPtr* pcb_aux = (PCBPtr*)aux;
        PCB* pcb_to_wake=(PCB*)pcb_aux->pcb;
        List_detach(&waiting_list, (ListItem*) pcb_to_wake);
        pcb_to_wake->status=Ready;
        List_insert(&ready_list, ready_list.last, (ListItem*) pcb_to_wake);
        //disastrOS_printStatus();
        aux=aux->next;
      }
      printf("[RECEIVE] all blocked processes in ready\n");
    }
    printf("[RECEIVE %d] exiting\n",disastrOS_getpid());
  }
  
  return;
  
}
