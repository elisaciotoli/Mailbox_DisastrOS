#pragma once
#include "linked_list.h"
#include "disastrOS_resource.h"

typedef struct Mailbox{
 Resource resource;
 ListHead messages_list;
 ListHead waiting_list;
} Mailbox;

typedef struct Message{
 ListItem list;
 char* text;
 int size;
}Message;

void Mailbox_init();
Mailbox* Mailbox_alloc(int id, int type);
int Mailbox_free(Mailbox* mailbox);

void Message_init();
Message* Message_alloc(char* text);
int Message_free(Message* message);