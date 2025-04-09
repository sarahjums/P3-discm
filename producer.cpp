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

void sendVideo(const string& fileName) {
    try {
        io_context io_context;
        ip::tcp::socket socket(io_context);
        ip::tcp::endpoint endpoint(ip::make_address(SERVER_IP), PORT);
        socket.connect(endpoint);

        //read and send the file in chunks
        ifstream file(fileName, ios::binary);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << fileName << endl;
            return;
        }

        //send file names first
        string name = boost::filesystem::path(fileName).filename().string();
        char nameBuffer[256] = {0};
        strncpy(nameBuffer, name.c_str(), sizeof(nameBuffer) - 1);
        write(socket, boost::asio::buffer(nameBuffer, sizeof(nameBuffer)));

        //send file data
        char buffer[1024];
        while (file.read(buffer, 1024) || file.gcount() > 0) {
            write(socket, boost::asio::buffer(buffer, file.gcount()));
        }

        file.close();
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);

        //receive a reply
        char response[64] = {0};
        socket.read_some(boost::asio::buffer(response));
        string reply(response);
        cout << "Server reply: " << reply;

        if (reply.find("DUPLICATE") != string::npos) {
            cout << "Duplicate file detected. Skipping: " << fileName << endl;
            boost::filesystem::remove(fileName); 
        }

        if (reply.find("QUEUE_FULL") != string::npos) {
            cout << "Queue is Full. Upload Failed: " << fileName << endl;
            boost::filesystem::remove(fileName); 
        }

        if (reply.find("RECEIVED") != string::npos) {
            cout << "Sent: " << fileName << endl;
            boost::filesystem::remove(fileName); //remove vid from folder after sending
        }

    } catch (std::exception& e) {
        cerr << "Error sending: " << e.what() << endl;
    }
}

void producerThread(const string& folderName) {
    while (true) {
        for (const auto& entry : boost::filesystem::directory_iterator(folderName)) {
            if (is_regular_file(entry.path())) {
                sendVideo(entry.path().string());
            }
        }
        this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int main() {
    int p;
    cout << "Enter number of producer threads: ";
    cin >> p;

    for (int i = 0; i < p; ++i) {
        string folder = "videos" + to_string(i + 1);
        if (!boost::filesystem::exists(folder))
            boost::filesystem::create_directory(folder);
    }

    vector<thread> producers;
    for (int i = 0; i < p; ++i) {
        producers.emplace_back(producerThread, "videos" + to_string(i + 1));
    }

    for (auto& t : producers)
        t.join();

    return 0;
}
