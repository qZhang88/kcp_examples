# Example: send hello!

**Complie**
```bash
gcc kcp_server.cc -o server -lstdc++
gcc kcp_client.cc -o client -lstdc++
```


**Run**

Open two shell and starte server and client
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
UDP recv data: size = 29, buf =
data recv: ip = 127.0.0.1, port = 52448
[Conn]  Data from Client-> Conn
Server reply -> 内容[Conn-OK], 字节[8] ret = 0
第[1]次发
[Hello]  Data from Client-> Conn
udp_output: 56 bytes content: []
UDP recv data: size = 176, buf = Client: Hello!
data recv: ip = 127.0.0.1, port = 52448
[Hello]  Data from Client-> Client: Hello!
第[2]次发
udp_output: 176 bytes content: []
UDP recv data: size = 176, buf = Client: Hello!
data recv: ip = 127.0.0.1, port = 52448
[Hello]  Data from Client-> Client: Hello!
```


client:
```bash
This is kcp client
client socket: sockfd = 3, server ip = 127.0.0.1  port = 10000
ikcp_send: [Conn] len=5, ret = 0.
udp_output: 29 bytes content: [Conn]
UDP recv data: size = 56, buf = Conn-OK
data recv: ip = 127.0.0.1, port = 10000
Data from Server-> Conn-OK
Client reply -> 内容[Client: Hello!], 字节[128]  ret = 0
第[1]次发
!!! Data from Server-> Conn-OK
udp_output: 176 bytes content: []
UDP recv data: size = 176, buf = Server: Hello!
data recv: ip = 127.0.0.1, port = 10000
Data from Server-> Server: Hello!
第[2]次发
udp_output: 176 bytes content: []
UDP recv data: size = 176, buf = Server: Hello!
data recv: ip = 127.0.0.1, port = 10000
Data from Server-> Server: Hello!
```


**NOTE**

First udp length is 24 + data len, if kcp kept been recv it will become 24 + 24 + data len, unless send not recieved, like if you killed client first, server will output like this.

```bash
第[3]次发
udp_output: 176 bytes content: []
udp_output: 152 bytes content: [Server: Hello!]
udp_output: 152 bytes content: [Server: Hello!]
udp_output: 152 bytes content: [Server: Hello!]
udp_output: 152 bytes content: [Server: Hello!]
udp_output: 152 bytes content: [Server: Hello!]
```
