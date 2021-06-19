#pragma once
#include "linked_list.h"
#include "disastrOS_resource.h"

typedef struct Mailbox{
 ListItem list;
 Resource* resource;
 ListHead messages_list;
} Mailbox;

typedef struct Message{
 ListItem list;
 char* text;
 int size;
}Message;

void Mailbox_init();
Mailbox* Mailbox_alloc(int id, int type);
int Mailbox_free(Mailbox* mailbox);
typedef ListHead MailboxList;
Mailbox* MailboxList_byId(MailboxList* l, int id);
//void MailboxList_print(ListHead* l);

void Message_init();
Message* Message_alloc(char* text);
int Message_free(Message* message);