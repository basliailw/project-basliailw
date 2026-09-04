#include "Message.hpp"

Message::Message(MessageRole role, std::string text) {
    // TODO: implement according to the M1 specification.
    role_ = role;
    text_ = text;
}

bool Message::operator==(const Message& other) const {
    // TODO
    return role_ == other.role_ &&
           text_ == other.text_;
}

bool Message::operator!=(const Message& other) const {
    // TODO
    return !(*this == other);
}

MessageRole Message::role() const noexcept {
    // TODO
    return role_;
}

const std::string& Message::text() const noexcept {
    // TODO
    return text_;
}

void Message::setRole(MessageRole role) noexcept {
    // TODO
    role_ = role;
}

void Message::setText(std::string text) {
    // TODO
    text_ = text;
}

bool Message::empty() const noexcept {
    // TODO
    return text_ == "";
}
