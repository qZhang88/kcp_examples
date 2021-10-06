# Example: KCP Image Streaming!

Using kcp with opencv to streaming camera video from client to server.

**Complie**
```bash
mkdir build
cd build

cmake ..
make
```


**Run**

Open two shell and start server and client
```bash
./server 10000
```

```bash
./client 127.0.0.1 10000
```

You will find two window for send and recv each.
