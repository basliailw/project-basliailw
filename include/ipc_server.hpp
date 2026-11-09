#pragma once

#include <QObject>
#include <atomic>
#include <string>
#include <thread>

namespace aiws::gui {

// /tmp/aiws-$USER.sock, or /tmp/aiws-<uid>.sock
std::string default_socket_path();

// 'A' | 4-byte big-endian length | path.  Reply: status (0 ok, 1 rejected) | length | message
class IpcServer {
public:
    IpcServer() = default;
    ~IpcServer();  // calls stop()
    IpcServer(const IpcServer&) = delete;
    IpcServer& operator=(const IpcServer&) = delete;

    // starts listening; each request calls slot_name(QString path) on receiver (GUI thread)
    bool start(const std::string& socket_path, QObject* receiver, const char* slot_name);

    // stops the thread, joins it, and removes the socket file
    void stop();

private:
    void run(int listen_fd, QObject* receiver, std::string slot_name);  // server thread
    void handle_client(int fd, QObject* receiver, const std::string& slot_name);
    bool read_exact(int fd, char* buffer, std::size_t n, long long deadline_ms);

    std::thread thread_;
    std::atomic<bool> stop_requested_{false}; // GUI writes, server thread reads
    int listen_fd_{-1}; // GUI thread only
    std::string socket_path_; // GUI thread only
};

}  // namespace aiws::gui