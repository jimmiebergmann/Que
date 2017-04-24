# Que
Minimal, but fast message queue, written in C++.
Distribute messages and get back respones via 4 commands.

## Protocol
Communication is done via TCP/IP. Pushed messages are destroyed if the producer disconnects.
### Requests
* PUSH <message size>\n<message>\n
* PULL\n
* ACK <message_size>\n<message>\n
* ABORT\n

#### Clarification
* ack: Acknowledging.
* producer - Client pushing messages.
* consumer - Client pulling messages.

### Respones
* PUSH
ACK <message_size>\n<message>\n - Acknowledgement from consimer.
TTL\n - The message reached time to live timeout.
* PULL
PULLING <message_size>\n<message>\n
* ACK
ACKED\n - The message has been acknowledged.
TOO_LATE\n - The consumer acknowledged too late.
The message has been destroyed or pulled by another consumer.

### Examples
#### Push - Send
```sh
PUSH 12\nHello world!\n
```
```sh
HEX: 50 55 53 48 20 31 32 0A 48 65 6C 6C 6F 20 77 6F 72 6C 64 21 0A
```

#### Pull - Receive

```sh
PULLING 12\nHello world!\n
```
```sh
HEX: 50 55 4C 4C 49 4E 47 20 31 32 0A 48 65 6C 6C 6F 20 77 6F 72 6C 64 21 0A
```