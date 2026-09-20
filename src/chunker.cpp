#include "aiws/chunker.hpp"
#include "aiws/text_processor.hpp"

#include <stdexcept>
#include <string>

namespace aiws {

Chunker::Chunker(ChunkingPolicy policy) : policy_(policy) {
    if (policy_.max_tokens == 0 || policy_.overlap >= policy_.max_tokens ||
        policy_.paragraph_window > policy_.max_tokens) {
        throw std::invalid_argument("invalid chunking policy");
    }
}

std::vector<Chunk> Chunker::chunk(const Document& document, std::size_t document_order) const {
    // TODO: produce deterministic, source-attributed chunks for the supplied document.
    std::vector<Chunk> chunks;
    const std::vector<TokenInfo> tokens = TextProcessor::tokenize(document.text());
    const std::size_t total = tokens.size();
    if (total == 0)
        return chunks;
    const std::size_t max_tokens = policy_.max_tokens;
    const std::size_t overlap = policy_.overlap;
    const std::size_t window_start_offset = (policy_.paragraph_window < max_tokens) ? (max_tokens - policy_.paragraph_window) : 0;
    std::size_t start = 0;
    std::size_t sequence = 0;
    while (start < total) {
        const std::size_t remaining = total - start;
        std::size_t end;
        if (remaining <= max_tokens)
            end = total;
        else {
            const std::size_t hard_end = start + max_tokens;
            end = hard_end;
            const std::size_t lo = start + window_start_offset;
            for (std::size_t c = hard_end ; c >= lo ; c--) {
                if (c >= 1 && c < total &&
                    tokens[c].paragraph != tokens[c - 1].paragraph) {
                    end = c;
                    break;
                }
                if (c == lo) break;
            }
        }
        Chunk chunk;
        chunk.id = document.id() + "#" + std::to_string(sequence);
        chunk.document_id = document.id();
        chunk.document_order = document_order;
        chunk.sequence = sequence;
        chunk.text = TextProcessor::join(tokens, start, end);
        chunk.token_count = end - start;
        chunk.source_begin = tokens[start].begin;
        chunk.source_end = tokens[end - 1].end;
        chunks.push_back(std::move(chunk));
        if (end >= total)
            break;
        start = (end > overlap) ? (end - overlap) : 0;
        ++sequence;
    } return chunks;
}

}  // namespace aiws
