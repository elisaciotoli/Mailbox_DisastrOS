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
    // need to sleep
    printf("[RECEIVE] in wait \n");
    running->status=Waiting;
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
    List_insert(&mailbox->waiting_list,mailbox->waiting_list.last, (ListItem*) running);

    // pick the next
    PCB* next_running= (PCB*) List_detach(&ready_list, ready_list.first);
    next_running->status=Running;
    running=next_running;
    return;
  }

  printf("[RECEIVE] detaching message\n");
  
  Message* message = (Message*) List_detach(&mailbox->messages_list,mailbox->messages_list.first);
  char* text = message->text;
  printf("[RECEIVE] message received = %s\n",text);
  if(Message_free(message)<0)
    printf("[RECEIVE] Errore: Cannot free message!\n");

  if((&mailbox->waiting_list)->size > 0){
    printf("[RECEIVE] putting blocked processes in ready\n");
    ListItem* aux=(&mailbox->waiting_list)->first;
    while(aux){
      PCB* pcb = (PCB*) aux;
      pcb->status=Ready;
      List_insert(&ready_list, ready_list.last, (ListItem*) pcb);
      aux=aux->next;
    }
    printf("[RECEIVE] all blocked processes in ready\n");
  }
  
  return;
  
}
