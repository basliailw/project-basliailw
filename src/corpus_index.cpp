#include "aiws/corpus_index.hpp"
#include "aiws/text_processor.hpp"

#include <stdexcept>

namespace aiws {

CorpusIndex::CorpusIndex(const std::vector<Chunk>& chunks) {
    build(chunks);
}

void CorpusIndex::build(const std::vector<Chunk>& chunks) {
    // TODO: build the searchable index from the supplied chunks.
    postings_.clear();
    chunk_by_id_.clear();
    for (std::size_t i = 0 ; i < chunks.size() ; i++) {
        const Chunk& c = chunks[i];
        chunk_by_id_[c.id] = i;
        const std::vector<std::string> chunk_terms = TextProcessor::terms(c.text);
        std::unordered_map<std::string, std::size_t> local_counts;
        for (const std::string& term : chunk_terms)
            local_counts[term]++;
        for (const auto& entry : local_counts)
            postings_[entry.first].push_back(Posting{i, entry.second});
    }
}

std::size_t CorpusIndex::document_frequency(
    const std::string& normalized_term) const noexcept {
    // TODO: return how many chunks contain the requested term.
    const auto it = postings_.find(normalized_term);
    if (it == postings_.end()) {
        return 0;
    }
    return it->second.size();
}

std::size_t CorpusIndex::term_frequency(
    const std::string& normalized_term,
    const std::string& chunk_id) const noexcept {
    // TODO: return the requested term's frequency in the specified chunk.
    const auto chunk_it = chunk_by_id_.find(chunk_id);
    if (chunk_it == chunk_by_id_.end())
        return 0;
    const auto term_it = postings_.find(normalized_term);
    if (term_it == postings_.end())
        return 0;
    const std::size_t target = chunk_it->second;
    for (const Posting& p : term_it->second)
        if (p.chunk_index == target)
            return p.frequency;
    return 0;
}

const std::vector<CorpusIndex::Posting>* CorpusIndex::postings(
    const std::string& normalized_term) const noexcept {
    // TODO: return the postings associated with the requested term.
    const auto it = postings_.find(normalized_term);
    if (it == postings_.end())
        return nullptr;
    return &it->second;
}

const Chunk* CorpusIndex::find_chunk(
    const std::vector<Chunk>& chunks,
    const std::string& chunk_id) const noexcept {
    // TODO: find the chunk identified by the requested chunk ID.
    const auto it = chunk_by_id_.find(chunk_id);
    if (it == chunk_by_id_.end() || it->second >= chunks.size())
        return nullptr;
    return &chunks[it->second];
}

std::size_t CorpusIndex::chunk_index(const std::string& chunk_id) const {
    // TODO: return the stored index of the requested chunk ID.
    const auto it = chunk_by_id_.find(chunk_id);
    if (it == chunk_by_id_.end())
        throw std::out_of_range("unknown chunk id: " + chunk_id);
    return it->second;
}

}  // namespace aiws
