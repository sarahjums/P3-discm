#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

#define PORT 8080
#define BUFFER_SIZE 1024

int max_queue = 5; // user input will update this later

queue<ip::tcp::socket*> socket_queue;
mutex mtx;
condition_variable cv;

// Function to save the incoming video file
void save_video(ip::tcp::socket socket) {
    try {
        char buffer[BUFFER_SIZE];
        string fileName = "uploads/video_" + to_string(rand() % 1000) + ".mp4"; //unique file name
        
        ofstream file(fileName, ios::binary);
        if (!file.is_open()) {
            cerr << "Failed to save file: " << fileName << "!\n";
            return;
        }

        boost::system::error_code error;
        size_t length;
        while ((length = socket.read_some(boost::asio::buffer(buffer), error)) > 0) {
            file.write(buffer, length);
        }

        cout << "Received and saved: " << fileName << endl;

        file.close();
        socket.close();
    } catch (exception& e) {
        cerr << "Error receiving file: " << e.what() << endl;
    }
}

void consumer_worker() {
    while (true) {
        ip::tcp::socket* socket = nullptr;

        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return !socket_queue.empty(); });

            socket = socket_queue.front();
            socket_queue.pop();
        }
        cout << "Processing video\n";
        save_video(socket);
        delete socket;
    }
}


int main() {
    io_context io_context;
    ip::tcp::acceptor acceptor(io_context, ip::tcp::endpoint(boost::asio::ip::address_v4::any(), PORT));

    int c_threads;

    cout << "Number of consumer threads: ";
    cin >> c_threads;

    cout << "Max queue size: ";
    cin >> max_queue;

    vector<thread> workers;
    for (int i = 0; i < c_threads; ++i) {
        workers.emplace_back(consumer_worker);
    }

    cout << "Consumer server now waiting for incoming connections..." << endl;
    
    while (true) {
        ip::tcp::socket socket(io_context);
        acceptor.accept(socket);

        cout << "Connection established with producer!"<< endl;

        unique_lock<mutex> lock(queue_mutex);
        if (socket_queue.size() < max_queue_size) {
            socket_queue.push(socket);
            cout << "Accepted connection. Queue size: " << socket_queue.size() << "out of" << max_queue_size << endl;
            cv.notify_one();
        } else {
            cout << "Queue full. Dropping incoming connection.\n";
            socket->close();
            delete socket;
        }
    }

    // join threads after
    for (auto& t : consumer_threads) {
        t.join();
    }

    return 0;
}

