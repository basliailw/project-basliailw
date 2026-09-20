#include "aiws/text_processor.hpp"

namespace aiws {
namespace {

bool is_token_char(unsigned char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >- '0' && c <= '9');
}

char to_lower_ascii(unsigned char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    } return static_cast<char>(c);
}

bool gap_has_paragraph_break(const std::string& text, std::size_t begin, std::size_t end) {
    bool seen_newline = false;
    bool only_whitespace_since_newline = true;
    for (std::size_t i = begin ; i < end ; i++) {
        const char ch = text[i];
        if(ch == '\n') {
            if (seen_newline && only_whitespace_since_newline)
                return true;
            seen_newline = true;
            only_whitespace_since_newline = true;
        } else if (ch == ' ' || ch == '\t' || ch == '\r') {
        } else
            only_whitespace_since_newline = false;
    } return false;
}

} // namespace

std::vector<TokenInfo> TextProcessor::tokenize(const std::string& text) {
    // TODO: produce normalized tokens with source and paragraph information.
    std::vector<TokenInfo> tokens;
    const std::size_t n = text.size();
    std::size_t i = 0;
    std::size_t paragraph = 0;
    bool emitted_any = false;
    bool pending_break = false;
 
    while (i < n) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        if (is_token_char(c)) {
            const std::size_t begin = i;
            std::string token;
            while (i < n && is_token_char(static_cast<unsigned char>(text[i]))) {
                token.push_back(to_lower_ascii(static_cast<unsigned char>(text[i])));
                i++;
            }
            const std::size_t end = i;
            if (emitted_any && pending_break)
                paragraph++;
            pending_break = false;
            TokenInfo info;
            info.token = std::move(token);
            info.begin = begin;
            info.end = end;
            info.paragraph = paragraph;
            tokens.push_back(std::move(info));
            emitted_any = true;
        } else {
            const std::size_t gap_begin = i;
            while (i < n && !is_token_char(static_cast<unsigned char>(text[i]))) {
                i++;
            }
            if (gap_has_paragraph_break(text, gap_begin, i)) {
                pending_break = true;
            }
        }
    } return tokens;
}

std::vector<std::string> TextProcessor::terms(const std::string& text) {
    // TODO: return the normalized terms represented by the input text.
    const std::vector<TokenInfo> infos = tokenize(text);
    std::vector<std::string> out;
    out.reserve(infos.size());
    for (const TokenInfo& info : infos)
        out.push_back(info.token);
    return out;
}

std::string TextProcessor::normalize(const std::string& text) {
    // TODO: return the normalized form of the input text.
    const std::vector<TokenInfo> infos = tokenize(text);
    return join(infos, 0, infos.size());
}

std::string TextProcessor::join(const std::vector<TokenInfo>& tokens,
                                std::size_t begin,
                                std::size_t end) {
    // TODO: join the requested token range into normalized text.
    if (begin > tokens.size()) begin = tokens.size();
    if (end > tokens.size()) end = tokens.size();
    std::string out;
    for (std::size_t i = begin ; i < end ; i++) {
        if (i != begin) out.push_back(' ');
        out += tokens[i].token;
    }
    return out;
}

std::string TextProcessor::join(const std::vector<std::string>& tokens,
                                std::size_t begin,
                                std::size_t end) {
    // TODO: join the requested term range into normalized text.
    if (begin > tokens.size()) begin = tokens.size();
    if (end > tokens.size()) end = tokens.size();
    std::string out;
    for (std::size_t i = begin ; i < end ; i++) {
        if (i != begin) out.push_back(' ');
        out += tokens[i];
    }
    return out;
}

}  // namespace aiws
