#include "aiws/processing_core.hpp"
#include "aiws/chunker.hpp"
#include "aiws/context_builder.hpp"
#include "aiws/corpus_index.hpp"
#include "aiws/retrieval_engine.hpp"
#include "aiws/text_processor.hpp"

#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace aiws {

struct ProcessingCore::Impl {
    // TODO: define the internal state used by the processing core.
    Chunker chunker{ChunkingPolicy{ProcessingCore::kMaxChunkTokens,
                                   ProcessingCore::kChunkOverlap,
                                   ProcessingCore::kParagraphPreferenceWindow}};
    RetrievalEngine retrieval;
    ContextBuilder context_builder;
    std::vector<Chunk> chunks;
    CorpusIndex index;
};

ProcessingCore::ProcessingCore() : impl_(std::make_unique<Impl>()) { }

ProcessingCore::~ProcessingCore() = default;

ProcessingCore::ProcessingCore(ProcessingCore&&) noexcept = default;

ProcessingCore& ProcessingCore::operator=(ProcessingCore&&) noexcept = default;

std::string ProcessingCore::normalize(const std::string&) {
    // TODO: return the normalized form of the input text.
    return TextProcessor::normalize(text);
}

void ProcessingCore::rebuild(const Workspace& workspace) {
    // TODO: rebuild the processing state from the workspace.
    const std::vector<Document>& documents = workspace.documents();
    std::unordered_set<std::string> seen_ids;
    for (const Document& doc : documents)
        if (!seen_ids.insert(doc.id()).second)
            throw std::invalid_argument("rebuild: duplicate document id '" + doc.id() + "'");
    std::vector<Chunk> new_chunks;
    for (std::size_t order = 0; order < documents.size(); ++order) {
        std::vector<Chunk> doc_chunks =
            impl_->chunker.chunk(documents[order], order);
        for (Chunk& c : doc_chunks)
            new_chunks.push_back(std::move(c));
    }
    CorpusIndex new_index;
    new_index.build(new_chunks);
    impl_->chunks = std::move(new_chunks);
    impl_->index = std::move(new_index);
}

const std::vector<Chunk>& ProcessingCore::chunks() const noexcept {
    // TODO: return the chunks currently stored by the processing core.
    return impl_->chunks;
}

std::size_t ProcessingCore::chunk_count() const noexcept {
    // TODO: return the number of stored chunks.
    return impl_->chunks.size();
}

std::size_t ProcessingCore::document_frequency(const std::string&) const {
    // TODO: return the document frequency for the requested term.
    const std::vector<std::string> tokens = TextProcessor::terms(term);
    if (tokens.empty())
        return 0;
    if (tokens.size() > 1)
        throw std::invalid_argument("document_frequency: term must normalize to a single token");
    return impl_->index.document_frequency(tokens.front());
}

std::size_t ProcessingCore::term_frequency(const std::string& term,
                                           const std::string& chunk_id) const {
    // TODO: return the term frequency for the requested chunk.
    const std::vector<std::string> tokens = TextProcessor::terms(term);
    if (tokens.empty())
        return 0;
    if (tokens.size() > 1)
        throw std::invalid_argument("term_frequency: term must normalize to a single token");
    return impl_->index.term_frequency(tokens.front(), chunk_id);
}

std::vector<SearchResult> ProcessingCore::search(const std::string& query, int k) const {
    // TODO: return the ranked results for the requested query.
    return impl_->retrieval.search(query, k, impl_->chunks, impl_->index);
}

std::vector<ContextItem> ProcessingCore::build_context(const std::string& query,
                                                       int k,
                                                       std::size_t token_budget) const {
    // TODO: build bounded context for the requested query.
    const std::vector<SearchResult> ranked = search(query, k);
    return impl_->context_builder.build(ranked, token_budget);
}

}  // namespace aiws
