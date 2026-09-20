#include "aiws/retrieval_engine.hpp"
#include "aiws/text_processor.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace aiws {

double RetrievalEngine::canonical_score(double value) {
    // TODO: return the score in the required canonical form.
    return std::round(value * 1e12) / 1e12;
}

std::vector<SearchResult> RetrievalEngine::search(const std::string& query,
                                                  int k,
                                                  const std::vector<Chunk>& chunks,
                                                  const CorpusIndex& index) const {
    // TODO: return the ranked search results for the requested query.
    if (k < 0)
        throw std::invalid_argument("search: k must be non-negative");
    std::vector<SearchResult> results;
    if (k == 0)
        return results;
    const std::vector<std::string> query_terms = TextProcessor::terms(query);
    std::vector<std::string> unique_terms;
    {
        std::unordered_set<std::string> seen;
        for (const std::string& t : query_terms)
            if (seen.insert(t).second)
                unique_terms.push_back(t);
    }
    const std::size_t q = unique_terms.size();
    if (q == 0)
        return results;
    const std::size_t n = chunks.size();
    struct Accumulator {
        double base = 0.0;
        std::size_t matched = 0;
    };
    std::unordered_map<std::size_t, Accumulator> acc;
    for (const std::string& term : unique_terms) {
        const std::vector<CorpusIndex::Posting>* postings = index.postings(term);
        if (postings == nullptr)
            continue;
        const std::size_t df = postings->size();
        const double idf =
            std::log(static_cast<double>(n + 1) / static_cast<double>(df + 1)) + 1.0;
        for (const CorpusIndex::Posting& p : *postings) {
            const double tf = 1.0 + std::log(static_cast<double>(p.frequency));
            Accumulator& a = acc[p.chunk_index];
            a.base += tf * idf;
            a.matched += 1;
        }
    }
    struct Scored {
        SearchResult result;
        std::size_t document_order;
        std::size_t sequence;
    };
    std::vector<Scored> scored;
    scored.reserve(acc.size());
    for (const auto& entry : acc) {
        const std::size_t idx = entry.first;
        if (idx >= n) continue;
        const Chunk& c = chunks[idx];
        const double coverage = 1.0 + 0.10 * static_cast<double>(entry.second.matched) / static_cast<double>(q);
        const double score = canonical_score(entry.second.base * coverage);
        SearchResult r;
        r.chunk_id = c.id;
        r.document_id = c.document_id;
        r.chunk_sequence = c.sequence;
        r.text = c.text;
        r.score = score;
        r.matched_terms = entry.second.matched;
        scored.push_back(Scored{std::move(r), c.document_order, c.sequence});
    }
    std::sort(scored.begin(), scored.end(), [](const Scored& a, const Scored& b) {
                  if (a.result.score != b.result.score)
                      return a.result.score > b.result.score;
                  if (a.document_order != b.document_order)
                      return a.document_order < b.document_order;
                  return a.sequence < b.sequence;});
    const std::size_t limit =
        std::min(static_cast<std::size_t>(k), scored.size());
    results.reserve(limit);
    for (std::size_t i = 0 ; i < limit ; i++)
        results.push_back(std::move(scored[i].result));
    return results;
}

}  // namespace aiws

