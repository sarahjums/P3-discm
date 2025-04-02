#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;

#define SERVER_IP "000.000.000.000"  // replace with ur own consumer machine's IP address
#define PORT 8080
#define BUFFER_SIZE 1024


void sendVideo(const string& fileName) {
    try {
        io_context io_context;
        ip::tcp::socket socket(io_context);
        ip::tcp::endpoint endpoint(ip::make_address(SERVER_IP), PORT);
        socket.connect(endpoint);
        
        ifstream file(fileName, ios::binary);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << fileName << "!\n";
            return;
        }

        //read and send the file in chunks
        char buffer[BUFFER_SIZE];
        while (file.read(buffer, BUFFER_SIZE) || file.gcount() > 0) {
            write(socket, boost::asio::buffer(buffer, file.gcount()));
        }

        cout << "Sent: " << fileName << "\n";

        file.close();
    } catch (std::exception& e) {
        cerr << "Error sending file: " << e.what() << endl;
    }
}

void producerThread(const string& folderName) {
    while(true){
        for (const auto& entry : boost::filesystem::directory_iterator(folderName)) {
            if (is_regular_file(entry.path())) {
            sendVideo(entry.path().string());

            cout << "Sent: " << entry.path().string() << "\n";
            
            remove(entry.path());  //remove vid from folder after sending
            }
        }
        this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int main() {
    // io_context io_context;
    // //create a socket to connect to the server
    // ip::tcp::socket socket(io_context);
    // ip::tcp::endpoint endpoint(ip::make_address(SERVER_IP), PORT);

    // socket.connect(endpoint);
    // cout << "Connected to Consumer Server!" << endl;

    int p;
    cout << "Enter number of producer threads: ";
    cin >> p;

    //automatically create p number of folders where videos could be uploaded
    for (int i = 0; i < p; ++i) {
        string folderName = "videos" + to_string(i + 1);
        if (!boost::filesystem::exists(folderName)) {
            boost::filesystem::create_directory(folderName);
            cout << "Created folder: " << folderName << endl;
        }
    }

    vector<thread> producers;
    for (int i = 0; i < p; ++i) {
        string folderName = "videos" + to_string(i + 1);
        producers.emplace_back(producerThread, folderName);
    }

    for (auto& t : producers) {
        t.join();
    }

    // socket.close();
    // cout << "Socket is closed. Program will exit! \n";

    return 0;
}
