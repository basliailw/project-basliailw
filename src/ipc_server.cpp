#include "ipc_server.hpp"

#include <QMetaObject>
#include <QString>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace aiws::gui {

namespace {

long long now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

// sends a reply: status byte, 4-byte big-endian length, message
void send_reply(int fd, char status, const std::string& message) {
    std::uint32_t len = static_cast<std::uint32_t>(message.size());
    std::string reply{status, char(len >> 24), char(len >> 16), char(len >> 8), char(len)};
    reply += message;
    ::send(fd, reply.data(), reply.size(), MSG_NOSIGNAL); // don't crash if aiws_send left
}

} // namespace

std::string default_socket_path() {
    const char* user = std::getenv("USER");
    if (user != nullptr && user[0] != '\0') return std::string("/tmp/aiws-") + user + ".sock";
    return "/tmp/aiws-" + std::to_string(::getuid()) + ".sock";
}

IpcServer::~IpcServer() {
    stop();
}

bool IpcServer::start(const std::string& socket_path, QObject* receiver, const char* slot_name) {
    sockaddr_un address{};
    if (thread_.joinable() || socket_path.size() >= sizeof(address.sun_path)) return false;
    address.sun_family = AF_UNIX;
    std::strcpy(address.sun_path, socket_path.c_str());

    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    ::unlink(socket_path.c_str()); // remove a leftover socket file
    if (fd < 0 || ::bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
        ::listen(fd, 8) != 0) {
        if (fd >= 0) ::close(fd);
        return false;
    }

    listen_fd_ = fd;
    socket_path_ = socket_path;
    stop_requested_ = false;
    thread_ = std::thread(&IpcServer::run, this, fd, receiver, std::string(slot_name));
    return true;
}

void IpcServer::stop() {
    if (!thread_.joinable()) return;
    stop_requested_ = true;
    thread_.join(); // the thread checks the flag every 100 ms
    ::close(listen_fd_);
    ::unlink(socket_path_.c_str());
}

void IpcServer::run(int listen_fd, QObject* receiver, std::string slot_name) {
    while (!stop_requested_) {
        pollfd waiting{listen_fd, POLLIN, 0};
        if (::poll(&waiting, 1, 100) <= 0) continue; // wake every 100 ms to check stop
        int client = ::accept(listen_fd, nullptr, nullptr);
        if (client < 0) continue;
        handle_client(client, receiver, slot_name);
        ::close(client);
    }
}

void IpcServer::handle_client(int fd, QObject* receiver, const std::string& slot_name) {
    long long deadline = now_ms() + 5000; // 5 seconds to send the whole request

    unsigned char header[5];
    if (!read_exact(fd, reinterpret_cast<char*>(header), 5, deadline)) return;
    std::uint32_t len = (header[1] << 24) | (header[2] << 16) | (header[3] << 8) | header[4];
    if (header[0] != 'A' || len == 0 || len > 65536) {
        send_reply(fd, 1, "bad request");
        return;
    }

    std::string path(len, '\0');
    if (!read_exact(fd, &path[0], len, deadline)) return;

    if (!std::ifstream(path).is_open()) { // check readability here, without the GUI
        send_reply(fd, 1, "cannot read file: " + path);
        return;
    }

    // queue the add for the GUI first, then reply
    QMetaObject::invokeMethod(receiver, slot_name.c_str(), Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromStdString(path)));
    send_reply(fd, 0, "ok");
}

bool IpcServer::read_exact(int fd, char* buffer, std::size_t n, long long deadline_ms) {
    std::size_t got = 0;
    while (got < n) {
        if (stop_requested_ || now_ms() > deadline_ms) return false;
        pollfd waiting{fd, POLLIN, 0};
        if (::poll(&waiting, 1, 100) <= 0) continue; // data can arrive in pieces
        ssize_t r = ::read(fd, buffer + got, n - got);
        if (r <= 0) return false; // aiws_send hung up
        got += static_cast<std::size_t>(r);
    }
    return true;
}

} // namespace aiws::gui