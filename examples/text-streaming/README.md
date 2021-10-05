# Example: send hello!

**Complie**
```bash
gcc kcp_server.cc -o server -lstdc++
gcc kcp_client.cc -o client -lstdc++
```


**Run**

Open two shell and start server and client
```bash
./server 10000
```

```bash
./client 127.0.0.1 10000
```

You will find printed masseges.

server:
```bash
This is kcp server
server socket: sockfd = 3  port:10000

UDP recv: size = 29, buf = Conn
KCP recv: ip = 127.0.0.1, port = 62741, buf size = 29
[Conn]  Data from Client-> Conn
Server reply -> 内容[Conn-OK], 字节[8] ret = 0

udp_output: 56 bytes content: []

UDP recv: size = 112, buf = [1]COMMAND:
KCP recv: ip = 127.0.0.1, port = 62741, buf size = 112
[1]: [1]COMMAND:
udp_output: 24 bytes content: []

UDP recv: size = 88, buf = [2]COMMAND:
KCP recv: ip = 127.0.0.1, port = 62741, buf size = 88
[2]: [2]COMMAND:
udp_output: 24 bytes content: []

UDP recv: size = 88, buf = [3]COMMAND:
KCP recv: ip = 127.0.0.1, port = 62741, buf size = 88
[3]: [3]COMMAND:
udp_output: 24 bytes content: []

UDP recv: size = 88, buf = [4]COMMAND:
KCP recv: ip = 127.0.0.1, port = 62741, buf size = 88
[4]: [4]COMMAND:
udp_output: 24 bytes content: []

UDP recv: size = 88, buf = [5]COMMAND:
KCP recv: ip = 127.0.0.1, port = 62741, buf size = 88
[5]: [5]COMMAND:
udp_output: 24 bytes content: []
```


client:
```bash
This is kcp client
client socket: sockfd = 3, server ip = 127.0.0.1  port = 10000
ikcp_send: [Conn] len=5, ret = 0.
udp_output: 29 bytes content: [Conn]
KCP recv: ip = 127.0.0.1, port = 10000, buf size = 56
Data from Server-> Conn-OK


第[1]次发
udp_output: 112 bytes content: []
KCP recv: ip = 127.0.0.1, port = 10000, buf size = 24

第[2]次发
udp_output: 88 bytes content: [[2]COMMAND: ]
KCP recv: ip = 127.0.0.1, port = 10000, buf size = 24

第[3]次发
udp_output: 88 bytes content: [[3]COMMAND: ]
KCP recv: ip = 127.0.0.1, port = 10000, buf size = 24

第[4]次发
udp_output: 88 bytes content: [[4]COMMAND: ]
KCP recv: ip = 127.0.0.1, port = 10000, buf size = 24

第[5]次发
udp_output: 88 bytes content: [[5]COMMAND: ]
KCP recv: ip = 127.0.0.1, port = 10000, buf size = 24
```
