#include <iostream>
#include <fstream>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

#define PORT 8080
#define BUFFER_SIZE 1024

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

int main() {
    io_context io_context;
    ip::tcp::acceptor acceptor(io_context, ip::tcp::endpoint(boost::asio::ip::address_v4::any(), PORT));

    cout << "Consumer server now waiting for incoming connections..." << endl;

    
    while (true) {
        ip::tcp::socket socket(io_context);
        acceptor.accept(socket);

        cout << "Connection established with producer!"<< endl;

        save_video(std::move(socket));
    }

    return 0;
}
