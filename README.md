# P3-discm

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
## SAMPLE DEMO


https://github.com/user-attachments/assets/db1a3683-4ef7-4ae1-bead-b9ccc9b50585



