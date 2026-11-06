#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

namespace {

std::string default_socket_path() {
    const char* user = std::getenv("USER");
    if (user != nullptr && user[0] != '\0') {
        return std::string("/tmp/aiws-") + user + ".sock";
    }
    return "/tmp/aiws-" + std::to_string(::getuid()) + ".sock";
}

void print_usage() {
    std::cerr << "usage: aiws_send [--socket PATH] add FILE\n";
}

// Keeps trying to connect for up to 2 seconds
// returns socket, or -1 if nothing answered.
int connect_to_app(const std::string& socket_path) {
    sockaddr_un address{};
    if (socket_path.size() >= sizeof(address.sun_path))
        return -1;
    address.sun_family = AF_UNIX;
    std::strcpy(address.sun_path, socket_path.c_str());

    auto give_up_at = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (true) {
        int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd >= 0 && ::connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0) {
            return fd;
        }
        if (fd >= 0) ::close(fd);
        if (std::chrono::steady_clock::now() >= give_up_at) {
            return -1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

bool send_all(int fd, const std::string& data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = ::send(fd, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (n <= 0) return false;  // includes hitting the 5 second timeout
        sent += static_cast<std::size_t>(n);
    }
    return true;
}

bool read_exact(int fd, char* buffer, std::size_t n) {
    std::size_t got = 0;
    while (got < n) {
        ssize_t r = ::read(fd, buffer + got, n - got);
        if (r <= 0) return false;  // includes hitting the 5 second timeout
        got += static_cast<std::size_t>(r);
    }
    return true;
}

void put_length(std::string& out, std::uint32_t len) {
    out += static_cast<char>((len >> 24) & 0xFF);
    out += static_cast<char>((len >> 16) & 0xFF);
    out += static_cast<char>((len >> 8) & 0xFF);
    out += static_cast<char>(len & 0xFF);
}

std::uint32_t get_length(const char* bytes) {
    return (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[0])) << 24) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[1])) << 16) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[2])) << 8) |
           static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[3]));
}

}  // namespace

int main(int argc, char* argv[]) {
    // Read the command line: [--socket PATH] add FILE
    std::string socket_path = default_socket_path();
    int i = 1;
    if (i + 1 < argc && std::string(argv[i]) == "--socket") {
        socket_path = argv[i + 1];
        i += 2;
    }
    if (i + 2 != argc || std::string(argv[i]) != "add") {
        print_usage();
        return 2;
    }

    std::string path = std::filesystem::absolute(argv[i + 1]).string();
 
    int fd = connect_to_app(socket_path);
    if (fd < 0) {
        std::cerr << "aiws_send: no app is listening on " << socket_path << "\n";
        return 2;
    }

    // give every send/read 5 seconds before calling it a communication failure.
    timeval timeout{5, 0};
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    std::string request = "A";
    put_length(request, static_cast<std::uint32_t>(path.size()));
    request += path;

    char header[5];
    if (!send_all(fd, request) || !read_exact(fd, header, 5)) {
        std::cerr << "aiws_send: lost contact with the app\n";
        ::close(fd);
        return 2;
    }
    std::string message(get_length(header + 1), '\0');
    if (!message.empty() && !read_exact(fd, &message[0], message.size())) {
        std::cerr << "aiws_send: lost contact with the app\n";
        ::close(fd);
        return 2;
    }
    ::close(fd);
    if (header[0] == 0)
        return 0;
    std::cerr << "aiws_send: rejected: " << message << "\n";
    return 1;
}