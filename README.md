# STDISCM Problem Set 3

## DEMO
https://github.com/user-attachments/assets/58828ea0-c520-4922-9ef3-e55053c03461

- (0:03) Network Setup
- (1:37) Threading Implementation of Producer Process
- (1:58) Producer Process
- (1:58) How the Consumer Process Receive the Files
- (2:58) Threading Implementation of Consumer Process
- (3:49) How Uploads Are Limited
- (4:38) Video Preview on Hover
- (4:55) Video Playback
##### Bonus Features:
- (3:56) Consumer tells producer that queue is full 
- (5:15) Duplicate Detection 
- (5:59) Video Compression 



## Requirements
Befor running the project, make sure you have the following installed:
- GCC/G++ compiler on both machines
- Boost.Asio (for networking) and Boost.Filesystem (for file handling) on both machines 
- OpenSSL (for hashing) on consumer machine
- FFmpeg (for video compression) on consumer machine

## On Consumer Side 
1. Compile & run `consumer.cpp` using the appropriate command for your OS.  

```sh
g++ consumer.cpp -o consumer -lboost_system -lboost_filesystem -lssl -lcrypto -std=c++11
./consumer
```  

2. In a new terminal, compile & run `server.cpp` to view GUI (web browser based).
```sh
g++ server.cpp -o server -lboost_system -lboost_filesystem -std=c++11
./server
```  

3. In your browser, go to:
```sh
http://localhost:8081/
```  

## On Producer Side (Ubuntu Virtual Machine)  
4. **Modify `producer.cpp`:** Change `SERVER_IP` to your consumer machine’s IP address.  
```sh
#define SERVER_IP "000.000.000.000"  // replace
```  
5. **Compile & Run `producer.cpp`:** 

```sh
g++ producer.cpp -o producer -lboost_system -lboost_filesystem -pthread -std=c++11
./producer
```


