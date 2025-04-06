#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

#define WEB_PORT 8080
#define WEB_ROOT "./gui"

string getMime(const string& path) {
    if (path.find(".mp4") != string::npos) return "video/mp4";
    if (path.find(".html") != string::npos) return "text/html";
    return "application/octet-stream";
}

void serve(tcp::socket socket) {
    try {
        boost::system::error_code error;
        char data[1024];
        size_t len = socket.read_some(buffer(data), error);
        string request(data, len);

        istringstream ss(request);
        string method, path;
        ss >> method >> path;

        if (path == "/") path = "/index.html";
        string fullPath = WEB_ROOT + path;

        ifstream file(fullPath, ios::binary);
        if (!file) {
            string response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
            write(socket, buffer(response));
            return;
        }

        stringstream fileContent;
        fileContent << file.rdbuf();
        string content = fileContent.str();

        string mime = getMime(fullPath);
        string header = "HTTP/1.1 200 OK\r\nContent-Type: " + mime + "\r\nContent-Length: " + to_string(content.size()) + "\r\n\r\n";
        write(socket, buffer(header + content));
    } catch (...) {
        // ignore socket errors
    }
}

int main() {
    io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), WEB_PORT));

    cout << "Web server running on port " << WEB_PORT << endl;

    while (true) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        thread(serve, std::move(socket)).detach();
    }

    return 0;
}
