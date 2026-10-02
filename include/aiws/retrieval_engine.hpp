#pragma once

#include "aiws/corpus_index.hpp"
#include "aiws/processing_types.hpp"
#include "aiws/retrieval_strategy.hpp"

#include <string>
#include <vector>

namespace aiws {

class RetrievalEngine : public RetrievalStrategy {
public:
    std::vector<SearchResult> search(const std::string& query,
                                     int k,
                                     const std::vector<Chunk>& chunks,
                                     const CorpusIndex& index) const override;

private:
    static double canonical_score(double value);
};

}  // namespace aiws
