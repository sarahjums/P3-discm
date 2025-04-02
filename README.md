# P3-discm

## On Consumer Side (Mac/Windows Machine)  
Compile & run `consumer.cpp` using the appropriate command for your OS.  

For Mac:  
```sh
g++ consumer.cpp -o consumer -lboost_system -std=c++11
./consumer
```  

For Windows:  
```sh
g++ consumer.cpp -o consumer -lboost_system -lws2_32 -std=c++11
./consumer
```  

## On Producer Side (Ubuntu Virtual Machine)  
1. **Modify `producer.cpp`:** Change `SERVER_IP` to your consumer machine’s IP address.  
2. **Compile & Run `producer.cpp`:** 

For Mac:
   ```sh
   g++ producer.cpp -o producer -lboost_system -lboost_filesystem -pthread -std=c++11
  ./producer
   ```  
For Windows:
   ```sh
   g++ producer.cpp -o producer -lboost_system -lboost_filesystem -lws2_32 -std=c++11
  ./producer
   ```  
3. **Enter the Number of Producer Threads.**  
4. **Start Adding Videos** to the created folders and verify that the videos are sent and received on the consumer side.  



https://github.com/user-attachments/assets/e9927438-57c4-4568-a53e-213c63405a43

