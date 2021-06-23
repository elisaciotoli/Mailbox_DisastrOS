# Mailbox in DisastrOS

### _IPC system based on message queues to allow blocking communication between processes_ ###


The project implements an IPC system based on message queues in DisastrOS to allow blocking communication between processes.

DisastrOS has been modified in order to support the possibility for processes to use mailboxes with the following structure:

```c
typedef struct Mailbox{
 Resource resource;
 ListHead messages_list;
 ListHead waiting_list;
} Mailbox;
```
Mailboxes can be opened and used through their 'id'. The 'id' is one of the fields of the Resource structure:

``` c
typedef struct {
  ListItem list;
  int id;
  int type;
  ListHead descriptors_ptrs;
} Resource;
```

The 'type' of the mailbox resource can be set in disastrOS_constants.h file modifying the following constant:
```c
#define MAILBOX_TYPE 1
```


Processes can exchange messages through a mailbox.
> Sending a message through a mailbox will block the process if the message queue is full.

> Receiving a message from a mailbox will block the process if the message queue is empty.

DisastrOS manages the send and the receive calls using the following structure for messages:

```c
typedef struct Message{
 ListItem list;
 char* text;
 int size;
} Message;
```


## Functionalities

> The OS provides any process the system calls to manage and properly use a mailbox. Let's suppose that 'mailbox_id' is the 'id' of the mailbox our process wants to use.

- [Create] - Create the mailbox and obtain a descriptor 'fd' for it
```c
int fd = disastrOS_openResource(mailbox_id,MAILBOX_TYPE,DSOS_CREATE);
```
- [Open] - Open the mailbox and obtain a descriptor 'fd' for it
```c
int fd = disastrOS_openResource(mailbox_id,MAILBOX_TYPE,0);
```
- [Send] - Send the message "message" through the mailbox
```c
disastrOS_send(mailbox_id,"message");
```
- [Receive] - Receive a message from the mailbox (and save it in the buffer 'buffer')
```c
disastrOS_receive(mailbox_id,&buffer);
```
- [Close] - Close the descriptor 'fd' of the mailbox
```c
disastrOS_closeResource(fd);
```
- [Destroy] - Destroy the mailbox
```c
disastrOS_destroyResource(mailbox_id)
```

## Where to find what ##

1. the mailbox structures and allocation files: 
   - disastrOS_mailbox.*

2. modified system calls to manage mailbox files:
   - disastrOS_open_resource.c
   - disastrOS_close_resource.c
   - disastrOS_destroy_resource.c

3. new system calls to manage mailbox files:
   - disastrOS_send.c
   - disastrOS_receive.c


## How-to-run

The following code allows to run the test program:

```sh
> make
> ./disastrOS_test
```

> The test program creates MAX_NUM_MAILBOXES mailboxes and spawns MAX_SENDERS senders and MAX_RECEIVERS receivers for each mailbox. Each sender/receiver sends/receives NUM_MESSAGES_SENDER/NUM_MESSAGES_RECEIVER messages.

> The constants are defined (and can be modified) in disastrOS_constants.h file:
```c
#define MAX_NUM_MAILBOXES 8
#define MAX_SENDERS 10
#define MAX_RECEIVERS 10
#define NUM_MESSAGES_SENDER 130
#define NUM_MESSAGES_RECEIVER 130
```

> The test program succeeds if the number of messages sent is equal to the number of messages received (total score = 0).

>In order to follow step by step how the test runs the following constant has to be redefined with a value different from 0.
```c
#define _PRINTFUL_ 0
```
