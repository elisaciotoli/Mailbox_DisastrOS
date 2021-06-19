#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_resource.h"
#include "disastrOS_mailbox.h"

void internal_send() {
  int mailbox_id = running->syscall_args[0];
  char* text = (char*)running->syscall_args[1];

  Mailbox* mailbox=MailboxList_byId(&mailboxes_list, mailbox_id);
  if(mailbox == NULL){
    printf("[SEND] Error: Cannot find mailbox!\n");
    return;
  }
  while(message_counter >= MAX_NUM_MESSAGES){
    //wait
    printf("[SEND] mailbox is full\n");
    disastrOS_sleep(5);
  }
  printf("[SEND] inserting message\n");
  Message* mes = Message_alloc(text);
  List_insert(&mailbox->messages_list,mailbox->messages_list.last,(ListItem*) mes);
  message_counter++;
  printf("[SEND] message inserted = %s\n",text);
  return;

}
