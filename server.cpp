#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

#define WEB_PORT 8081
#define VIDEO_ROOT "./videos" //folder where the vids are stored

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
        if (path == "/index.html") {
            stringstream videoList;
            videoList << "<html><body><h1>Uploaded Videos</h1><ul>";

            //list all .mp4 files
            for (const auto& entry : boost::filesystem::directory_iterator(VIDEO_ROOT)) {
                if (boost::filesystem::is_regular_file(entry) && entry.path().extension() == ".mp4") {
                    videoList << "<li><video width='320' height='240' controls "
                            << "onmouseover='this.currentTime=0; this.play(); setTimeout(() => this.pause(), 10000);' "
                            << "onmouseout='this.pause();' "
                            << "onclick='this.play();' ><source src='/videos/"
                            << entry.path().filename().string() << "' type='video/mp4'></video></li>";
                }
            }
            videoList << "</ul></body></html>";
            string content = videoList.str();

            string mime = getMime(path);
            string header = "HTTP/1.1 200 OK\r\nContent-Type: " + mime + "\r\nContent-Length: " + to_string(content.size()) + "\r\n\r\n";
            write(socket, buffer(header + content));
            return;
        }

        if (path.find("/videos/") == 0) {
            string videoPath = VIDEO_ROOT + path.substr(7);

            ifstream file(videoPath, ios::binary);
            if (!file) {
                string response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
                write(socket, buffer(response));
                return;
            }

            stringstream fileContent;
            fileContent << file.rdbuf();
            string content = fileContent.str();

            string mime = getMime(videoPath);
            string header = "HTTP/1.1 200 OK\r\nContent-Type: " + mime + "\r\nContent-Length: " + to_string(content.size()) + "\r\n\r\n";
            write(socket, buffer(header + content));
            return;
        }

        string response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        write(socket, buffer(response));

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
