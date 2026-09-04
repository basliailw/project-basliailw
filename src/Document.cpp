#include "Document.hpp"

Document::Document(std::string title, std::string contents) {
    // TODO: implement according to the M1 specification.
    title_ = title;
    contents_ = contents;
}

bool Document::operator==(const Document& other) const {
    // TODO
    return title_      == other.title_ &&
           contents_   == other.contents_ &&
           sourcePath_ == other.sourcePath_;
}

bool Document::operator!=(const Document& other) const {
    // TODO
    return !(*this == other);
}

bool Document::load(const std::string& path) {
    // TODO
    std::ifstream file(path);
    if (!file) {  return false; }
    std::string fileContents;
    char ch;
    while (file.get(ch)) { fileContents += ch; }
    std::size_t lastSlash = path.find_last_of('/');
    std::string filename;
    if (lastSlash == std::string::npos) { filename = path; }
    else { filename = path.substr(lastSlash + 1); }
    contents_ = fileContents;
    sourcePath_ = path;
    title_ = filename;
    return true;
}

const std::string& Document::title() const noexcept {
    // TODO
    return title_;
}

const std::string& Document::sourcePath() const noexcept {
    // TODO
    return sourcePath_;
}

const std::string& Document::contents() const noexcept {
    // TODO
    return contents_;
}

void Document::setTitle(std::string title) {
    // TODO
    title_ = title;
}

std::size_t Document::characterCount() const noexcept {
    // TODO
    return contents_.length();
}

bool Document::empty() const noexcept {
    // TODO
    return contents_ == "";
}
