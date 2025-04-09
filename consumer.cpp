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
#define SAVE_DIR "videos/"

int MAX_QUEUE_SIZE = 0;

mutex queue_mutex;
condition_variable cv;

struct VideoJob {
    vector<char> data;
    string hash;
    string fileName;
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
    string rawFile = SAVE_DIR + job.fileName + "_raw";
    string finalFile = SAVE_DIR + job.fileName;

    ofstream out(rawFile, ios::binary);
    if (!out.is_open()) {
        cerr << "Failed to open raw file for writing: " << rawFile << endl;
        return;
    }
    out.write(job.data.data(), job.data.size());
    out.close();

    string cmd = "ffmpeg -y -i " + rawFile + " -vcodec libx264 -crf 28 " + finalFile + " > /dev/null 2>&1";
    int ret = system(cmd.c_str());
    if (ret != 0) {
        cerr << "Error while compressing:" << cmd << endl;
        return;
    }

    // remove uncompressed file
    boost::filesystem::remove(rawFile);

    cout << "(---UPLOAD FINISHED---) - Video saved and compressed: " << finalFile << endl;

    //cout << "Saved: " << finalFile << endl;
}

void consumerThreadFunc() {
    while (true) {
        unique_lock<mutex> lock(queue_mutex);
        cv.wait(lock, [] { return !videoQueue.empty(); });

        VideoJob job = videoQueue.front();
        videoQueue.pop();

        cout << "(UPLOADING) - " << job.fileName << " Removed from queue. Updated queue size: " << videoQueue.size() << " / " << MAX_QUEUE_SIZE << endl;

        lock.unlock();

        saveAndCompressVideo(job);
    }
}

void handle_connection(tcp::socket socket) {
    try {
        vector<char> buffer;
        char temp[1024];
        boost::system::error_code error;
        size_t length;

        char nameBuffer[256] = {0};
        boost::asio::read(socket, boost::asio::buffer(nameBuffer, sizeof(nameBuffer)));
        string fileName(nameBuffer);

        while ((length = socket.read_some(boost::asio::buffer(temp), error)) > 0) {
            buffer.insert(buffer.end(), temp, temp + length);
            //cout << "Received " << length << " bytes of data\n";
        }

        string hash = computeHash(buffer);

        {
            lock_guard<mutex> lock(queue_mutex);

            // drop duplicates
            if (hashSet.count(hash)) {
                cout << "------> (DUPLICATE) Video dropped: " << fileName << " <------\n";
                write(socket, boost::asio::buffer("DUPLICATE\n"));
                return;
            }

            // leaky bucket
            if (videoQueue.size() >= MAX_QUEUE_SIZE) {
                cout << "------> (QUEUE FULL) Video dropped: " << fileName << " <------\n";
                write(socket, ::boost::asio::buffer("QUEUE_FULL\n"));
                return;
            }

            videoQueue.push({buffer, hash, fileName});
            hashSet.insert(hash);

            cout << "(VIDEO RECEIVED) - " << fileName << " Added to queue. Updated queue size: " << videoQueue.size() << " / " << MAX_QUEUE_SIZE << endl;

            cv.notify_one();
        }


        write(socket, boost::asio::buffer("RECEIVED\n"));
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

    cout << "Enter max queue size: ";
    cin >> MAX_QUEUE_SIZE;

    vector<thread> consumers;
    for (int i = 0; i < c; ++i)
        consumers.emplace_back(consumerThreadFunc);

    cout << "Consumer listening on port " << PORT << endl << endl;

    while (true) {
        tcp::socket socket(io_context);
        //cout << "Waiting for connection..." << endl;
        acceptor.accept(socket);
        //cout << "Connection established with producer!"<< endl;
        thread(handle_connection, std::move(socket)).detach();
    }

    return 0;
}
