#include <assert.h>
#include <stdio.h>
#include "disastrOS_resource.h"
#include "disastrOS_descriptor.h"
#include "pool_allocator.h"
#include "linked_list.h"
#include "disastrOS_mailbox.h"
#include "disastrOS_globals.h"

#define MAILBOX_SIZE sizeof(Mailbox)
#define MAILBOX_MEMSIZE (sizeof(Mailbox)+sizeof(int))
#define MAILBOX_BUFFER_SIZE MAX_NUM_MAILBOXES*MAILBOX_MEMSIZE

static char _mailboxes_buffer[MAILBOX_BUFFER_SIZE];
static PoolAllocator _mailboxes_allocator;

#define MESSAGE_SIZE sizeof(Message)
#define MESSAGE_MEMSIZE (sizeof(Message)+sizeof(int))
#define MESSAGE_BUFFER_SIZE MAX_NUM_MESSAGES*MESSAGE_MEMSIZE

static char _messages_buffer[MESSAGE_BUFFER_SIZE];
static PoolAllocator _messages_allocator;


void Mailbox_init(){
    int result=PoolAllocator_init(& _mailboxes_allocator,
				  MAILBOX_SIZE,
				  MAX_NUM_MAILBOXES,
				  _mailboxes_buffer,
				  MAILBOX_BUFFER_SIZE);
    assert(! result);
}

Mailbox* Mailbox_alloc(int id, int type){
  Mailbox* r=(Mailbox*) PoolAllocator_getBlock(&_mailboxes_allocator);
  if (!r)
    return 0;
  r->list.prev=r->list.next=0;
  r->resource=Resource_alloc(id,type);
  List_insert(&resources_list, resources_list.last, (ListItem*) r->resource);
  List_init(&r->messages_list);
  return r;
}

int Mailbox_free(Mailbox* r) {
  assert(r->messages_list.first==0);
  assert(r->messages_list.last==0);
  return PoolAllocator_releaseBlock(&_mailboxes_allocator, r);
}

Mailbox* MailboxList_byId(MailboxList* l, int id) {
  ListItem* aux=l->first;
  while(aux){
    Mailbox* r=(Mailbox*)aux;
    if (r->resource->id==id)
      return r;
    aux=aux->next;
  }
  return 0;
}



void Message_init(){
    int result=PoolAllocator_init(& _messages_allocator,
				  MESSAGE_SIZE,
				  MAX_NUM_MESSAGES,
				  _messages_buffer,
				  MESSAGE_BUFFER_SIZE);
    assert(! result);
}

Message* Message_alloc(char* text){
  Message* r=(Message*) PoolAllocator_getBlock(&_messages_allocator);
  if (!r)
    return 0;
  r->list.prev=r->list.next=0;
  r->text = text;
  r->size = sizeof(text);
  return r;
}

int Message_free(Message* r) {
  //assert(r->messages_list.first==0);
  //assert(r->messages_list.last==0);
  return PoolAllocator_releaseBlock(&_messages_allocator, r);
}