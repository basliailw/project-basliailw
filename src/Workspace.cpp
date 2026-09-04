#include "Workspace.hpp"

Workspace::Workspace(std::string name) {
    // TODO: implement according to the M1 specification.
    name_ = name;
}

bool Workspace::operator==(const Workspace& other) const {
    // TODO
    return name_      == other.name_      &&
           documents_ == other.documents_ &&
           prompts_   == other.prompts_   &&
           messages_  == other.messages_;
}

bool Workspace::operator!=(const Workspace& other) const {
    // TODO
    return !(*this == other);
}

const std::string& Workspace::name() const noexcept {
    // TODO
    return name_;
}

void Workspace::setName(std::string name) {
    // TODO
    name_ = name;
}

void Workspace::addDocument(Document document) {
    // TODO
    documents_.push_back(document);
}

void Workspace::addPrompt(Prompt prompt) {
    // TODO
    prompts_.push_back(prompt);
}

void Workspace::addMessage(Message message) {
    // TODO
    messages_.push_back(message);
}

std::size_t Workspace::documentCount() const noexcept {
    // TODO
    return documents_.size();
}

std::size_t Workspace::promptCount() const noexcept {
    // TODO
    return prompts_.size();
}

std::size_t Workspace::messageCount() const noexcept {
    // TODO
    return messages_.size();
}

Document& Workspace::documentAt(std::size_t index) {
    // TODO: replace this placeholder behavior.
    return documents_.at(index);
}

const Document& Workspace::documentAt(std::size_t index) const {
    // TODO: replace this placeholder behavior.
    return documents_.at(index);
}

Prompt& Workspace::promptAt(std::size_t index) {
    // TODO: replace this placeholder behavior.
    return prompts_.at(index);
}

const Prompt& Workspace::promptAt(std::size_t index) const {
    // TODO: replace this placeholder behavior.
    return prompts_.at(index);
}

Message& Workspace::messageAt(std::size_t index) {
    // TODO: replace this placeholder behavior.
    return messages_.at(index);
}

const Message& Workspace::messageAt(std::size_t index) const {
    // TODO: replace this placeholder behavior.
    return messages_.at(index);
}
