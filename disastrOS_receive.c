#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_globals.h"
#include "disastrOS_mailbox.h"

void internal_receive() {
  int mailbox_id=running->syscall_args[0];
  

  Mailbox* mailbox=MailboxList_byId(&mailboxes_list, mailbox_id);
  if(mailbox == NULL){
    printf("[RECEIVE] Error: Cannot find mailbox!\n");
    return;
  }
  while(message_counter <= 0){
    //wait
    printf("[RECEIVE] mailbox is empty\n");
    disastrOS_sleep(2);
  }
  printf("[RECEIVE] detaching message\n");
  
  Message* message = (Message*) List_detach(&mailbox->messages_list,mailbox->messages_list.first);
  char* text = message->text;
  message_counter--;
  printf("[RECEIVE] message received = %s\n",text);
  if(Message_free(message)<0)
    printf("[RECEIVE] Erroe: Cannot free message!\n");
  return;
  
}
