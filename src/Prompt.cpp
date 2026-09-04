#include "Prompt.hpp"

Prompt::Prompt(std::string title, std::string text) {
    // TODO: implement according to the M1 specification.
    title_ = title;
    text_ = text;
}

bool Prompt::operator==(const Prompt& other) const {
    // TODO
    return title_ == other.title_ &&
           text_  == other.text_;
}

bool Prompt::operator!=(const Prompt& other) const {
    // TODO
    return !(*this == other);
}

const std::string& Prompt::title() const noexcept {
    // TODO
    return title_;
}

const std::string& Prompt::text() const noexcept {
    // TODO
    return text_;
}

void Prompt::setTitle(std::string title) {
    // TODO
    title_ = title;
}

void Prompt::setText(std::string text) {
    // TODO
    text_ = text;
}

bool Prompt::empty() const noexcept {
    // TODO
    return text_ == "";
}
