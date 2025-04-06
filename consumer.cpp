#include <iostream>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>
#include <condition_variable>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <openssl/sha.h>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_QUEUE_SIZE 5
#define SAVE_DIR "videos/"

mutex queue_mutex;
condition_variable cv;

struct VideoJob {
    vector<char> data;
    string hash;
};

queue<VideoJob> videoQueue;
unordered_set<string> hashSet;

// compute SHA256 hash for video byte data to 
// detect duplicates
string computeHash(const vector<char>& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.data(), data.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];

    return ss.str();
}

// compress using ffmpeg
void saveAndCompressVideo(const VideoJob& job) {
    string rawFile = SAVE_DIR + job.hash + "_raw.mp4";
    string finalFile = SAVE_DIR + job.hash + ".mp4";

    ofstream out(rawFile, ios::binary);
    out.write(job.data.data(), job.data.size());
    out.close();

    string cmd = "ffmpeg -y -i " + rawFile + " -vcodec libx264 -crf 28 " + finalFile + " > /dev/null 2>&1";
    system(cmd.c_str());

    // remove uncompressed file
    boost::filesystem::remove(rawFile);

    cout << "Saved: " << finalFile << endl;
}

void consumerThreadFunc() {
    while (true) {
        unique_lock<mutex> lock(queue_mutex);
        cv.wait(lock, [] { return !videoQueue.empty(); });

        VideoJob job = videoQueue.front();
        videoQueue.pop();
        lock.unlock();

        saveAndCompressVideo(job);
    }
}

void handle_connection(tcp::socket socket) {
    try {
        vector<char> buffer;
        char temp[BUFFER_SIZE];
        boost::system::error_code error;
        size_t length;

        while ((length = socket.read_some(buffer(temp), error)) > 0) {
            buffer.insert(buffer.end(), temp, temp + length);
        }

        string hash = computeHash(buffer);

        {
            lock_guard<mutex> lock(queue_mutex);

            // drop duplicates
            if (hashSet.count(hash)) {
                cout << "Duplicate dropped\n";
                write(socket, buffer("DUPLICATE\n"));
                return;
            }

            // leaky bucket
            if (videoQueue.size() >= MAX_QUEUE_SIZE) {
                cout << "Queue full. Dropped\n";
                write(socket, buffer("QUEUE_FULL\n"));
                return;
            }

            videoQueue.push({buffer, hash});
            hashSet.insert(hash);
            cv.notify_one();
        }

        write(socket, buffer("RECEIVED\n"));
    } catch (exception& e) {
        cerr << "Connection error: " << e.what() << endl;
    }
}

int main() {
    if (!boost::filesystem::exists(SAVE_DIR))
        boost::filesystem::create_directory(SAVE_DIR);

    io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), PORT));

    int c;
    cout << "Enter number of consumer threads: ";
    cin >> c;

    vector<thread> consumers;
    for (int i = 0; i < c; ++i)
        consumers.emplace_back(consumerThreadFunc);

    cout << "Consumer listening on port " << PORT << endl;

    while (true) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        thread(handle_connection, std::move(socket)).detach();
    }

    return 0;
}
