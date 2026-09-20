#include "aiws/context_builder.hpp"
#include "aiws/text_processor.hpp"

#include <string>
#include <unordered_set>
#include <vector>

namespace aiws {

std::vector<ContextItem> ContextBuilder::build(const std::vector<SearchResult>& ranked, std::size_t token_budget) const {
    // TODO: assemble ranked results into context items within the supplied token budget.
    std::vector<ContextItem> items;
    if (token_budget == 0)
        return items;
    std::size_t used = 0;
    std::unordered_set<std::string> included;
    for (const SearchResult& r : ranked) {
        const std::size_t remaining = token_budget - used;
        if (remaining == 0)
            break;
        if (!included.insert(r.chunk_id).second)
            continue;
        const std::vector<std::string> tokens = TextProcessor::terms(r.text);
        const std::size_t chunk_tokens = tokens.size();
        ContextItem item;
        item.chunk_id = r.chunk_id;
        item.document_id = r.document_id;
        item.chunk_sequence = r.chunk_sequence;
        item.score = r.score;
        if (chunk_tokens <= remaining) {
            item.text = r.text;
            item.token_count = chunk_tokens;
            item.truncated = false;
            used += chunk_tokens;
            items.push_back(std::move(item));
        } else {
            item.text = TextProcessor::join(tokens, 0, remaining);
            item.token_count = remaining;
            item.truncated = true;
            used += remaining;
            items.push_back(std::move(item));
            break;
        }
    }
    return items;
}

}  // namespace aiws
