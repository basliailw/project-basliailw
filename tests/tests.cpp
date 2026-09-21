#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "aiws/chunker.hpp"
#include "aiws/context_builder.hpp"
#include "aiws/corpus_index.hpp"
#include "aiws/processing_core.hpp"
#include "aiws/retrieval_engine.hpp"
#include "aiws/text_processor.hpp"
#include "aiws/workspace.hpp"

#include <stdexcept>
#include <string>
#include <vector>

using namespace aiws;

static std::string words(int n) {
    std::string s;
    for (int i = 0 ; i < n ; i++) { if (i) s += ' '; s += "w" + std::to_string(i); }
    return s;
}

TEST_CASE("Text normalization", "[text_processor]") {
    REQUIRE(TextProcessor::normalize("Hello,  WORLD! 2026") == "hello world 2026");
    REQUIRE(TextProcessor::normalize("R2-D2") == "r2 d2");
    REQUIRE(TextProcessor::normalize("a,,,,b") == "a b");
    REQUIRE(TextProcessor::normalize("...\t---").empty());
    REQUIRE(TextProcessor::normalize("caf\xC3\xA9 bar") == "caf bar");
}

TEST_CASE("Tokenization: offsets and paragraphs", "[text_processor]") {
    SECTION("spans index the original text") {
        const auto t = TextProcessor::tokenize("  Hello,   world  ");
        REQUIRE(t.size() == 2);
        REQUIRE(t[0].begin == 2);
        REQUIRE(t[0].end == 7);
        REQUIRE(t[1].begin == 11);
        REQUIRE(t[1].end == 16);
    }
    SECTION("a blank line starts a new paragraph; a single newline does not") {
        const auto lf = TextProcessor::tokenize("a b\n\nc");
        REQUIRE(lf[2].paragraph == lf[1].paragraph + 1);
        const auto crlf = TextProcessor::tokenize("a b\r\n\r\nc");
        REQUIRE(crlf[2].paragraph == crlf[1].paragraph + 1);
        const auto one = TextProcessor::tokenize("a\nb");
        REQUIRE(one[0].paragraph == one[1].paragraph);
    }
}

TEST_CASE("Chunking: size, overlap, boundaries, identity", "[chunker]") {
    Chunker chunker;
    SECTION("empty document produces no chunks") {
        REQUIRE(chunker.chunk(Document{"e", "", "  \n\t"}, 0).empty());
    }
    SECTION("short document is one chunk with correct identity") {
        const auto c = chunker.chunk(Document{"d1", "", "alpha beta gamma"}, 3);
        REQUIRE(c.size() == 1);
        REQUIRE(c[0].id == "d1#0");
        REQUIRE(c[0].document_order == 3);
        REQUIRE(c[0].token_count == 3);
        REQUIRE(c[0].source_begin == 0);
        REQUIRE(c[0].source_end == 16);
    }
    SECTION("121 tokens -> 120 + 21 with 20-token overlap") {
        const auto c = chunker.chunk(Document{"L", "", words(121)}, 0);
        REQUIRE(c.size() == 2);
        REQUIRE(c[0].token_count == 120);
        REQUIRE(c[1].token_count == 21);
        REQUIRE(c[1].id == "L#1");
    }
    SECTION("paragraph boundary is preferred inside the 100-120 window only") {
        std::string in;
        for (int i = 0 ; i < 110 ; i++) { if (i) in += ' '; in += "a" + std::to_string(i); }
        in += "\n\n";
        for (int i = 110 ; i < 140 ; i++) { in += ' '; in += "b" + std::to_string(i); }
        REQUIRE(chunker.chunk(Document{"P", "", in}, 0)[0].token_count == 110);
        std::string out;
        for (int i = 0 ; i < 50 ; i++) { if (i) out += ' '; out += "a" + std::to_string(i); }
        out += "\n\n";
        for (int i = 50 ; i < 200 ; i++) { out += ' '; out += "b" + std::to_string(i); }
        REQUIRE(chunker.chunk(Document{"Q", "", out}, 0)[0].token_count == 120);
    }
}

TEST_CASE("CorpusIndex frequencies", "[corpus_index]") {
    std::vector<Chunk> chunks = {
        Chunk{"d1#0", "d1", 0, 0, "alpha alpha beta", 3, 0, 0},
        Chunk{"d2#0", "d2", 1, 0, "alpha gamma", 2, 0, 0}};
    CorpusIndex index(chunks);
    REQUIRE(index.document_frequency("alpha") == 2);
    REQUIRE(index.document_frequency("missing") == 0);
    REQUIRE(index.term_frequency("alpha", "d1#0") == 2);
    REQUIRE(index.term_frequency("alpha", "nope#9") == 0);
}

TEST_CASE("Rebuild is atomic and leaves no stale state",
          "[corpus_index][processing_core]") {
    Workspace ws;
    ws.add_document(Document{"d1", "", "alpha"});
    ws.add_document(Document{"d2", "", "beta"});
    ProcessingCore core;
    core.rebuild(ws);
    SECTION("repeated rebuild does not duplicate; removed doc leaves no term") {
        core.rebuild(ws);
        REQUIRE(core.chunk_count() == 2);
        Workspace smaller;
        smaller.add_document(Document{"d1", "", "alpha"});
        core.rebuild(smaller);
        REQUIRE(core.chunk_count() == 1);
        REQUIRE(core.document_frequency("beta") == 0);
    }
    SECTION("duplicate ids throw and preserve the previous corpus") {
        Workspace dup;
        dup.add_document(Document{"x", "", "one"});
        dup.add_document(Document{"x", "", "two"});
        REQUIRE_THROWS_AS(core.rebuild(dup), std::invalid_argument);
        REQUIRE(core.chunk_count() == 2);
        REQUIRE(core.document_frequency("alpha") == 1);
    }
    SECTION("term arguments are validated") {
        REQUIRE(core.document_frequency("!!!") == 0);
        REQUIRE_THROWS_AS(core.document_frequency("two words"),
                          std::invalid_argument);
    }
}

TEST_CASE("Retrieval ranking and determinism", "[retrieval]") {
    std::vector<Chunk> chunks = {
        Chunk{"d1#0", "d1", 0, 0, "alpha alpha beta", 3, 0, 0},
        Chunk{"d2#0", "d2", 1, 0, "alpha gamma", 2, 0, 0}};
    CorpusIndex index(chunks);
    RetrievalEngine engine;
    SECTION("candidate union, coverage ordering, and hand-computed scores") {
        const auto r = engine.search("alpha beta", 10, chunks, index);
        REQUIRE(r.size() == 2);
        REQUIRE(r[0].document_id == "d1");
        REQUIRE(r[0].matched_terms == 2);
        REQUIRE(r[0].score == Approx(3.4084735175364813));
        REQUIRE(r[1].score == Approx(1.05));
    }
    SECTION("k bounds, empty/unknown query, negative k") {
        REQUIRE(engine.search("alpha", 0, chunks, index).empty());
        REQUIRE(engine.search("", 10, chunks, index).empty());
        REQUIRE(engine.search("zzz", 10, chunks, index).empty());
        REQUIRE_THROWS_AS(engine.search("alpha", -1, chunks, index),
                          std::invalid_argument);
    }
    SECTION("ties break by insertion order then sequence, not by id") {
        std::vector<Chunk> tied = {
            Chunk{"d2#0", "d2", 0, 0, "gamma", 1, 0, 0},
            Chunk{"d1#0", "d1", 1, 0, "gamma", 1, 0, 0}};
        CorpusIndex ti(tied);
        const auto r = engine.search("gamma", 10, tied, ti);
        REQUIRE(r[0].score == Approx(r[1].score));
        REQUIRE(r[0].document_id == "d2");
    }
}

TEST_CASE("Bounded context respects the budget", "[context]") {
    ContextBuilder builder;
    std::vector<SearchResult> ranked = {
        SearchResult{"d1#0", "d1", 0, "alpha alpha beta", 2.0, 2},
        SearchResult{"d2#0", "d2", 0, "alpha gamma", 1.0, 1}};
    REQUIRE(builder.build(ranked, 0).empty());
    const auto trunc = builder.build(ranked, 2);
    REQUIRE(trunc.size() == 1);
    REQUIRE(trunc[0].token_count == 2);
    REQUIRE(trunc[0].truncated);
    REQUIRE(trunc[0].text == "alpha alpha");
    REQUIRE(trunc[0].document_id == "d1");
    const auto exact = builder.build(ranked, 3);
    REQUIRE(exact.size() == 1);
    REQUIRE_FALSE(exact[0].truncated);
}

TEST_CASE("End-to-end multi-document flow", "[integration]") {
    Workspace ws;
    ws.add_document(Document{"ai", "", "machine learning models learn from data"});
    ws.add_document(Document{"db", "", "database systems store and index data"});
    ws.add_document(Document{"net", "", "network protocols route packets"});
    ProcessingCore core;
    core.rebuild(ws);
    const auto results = core.search("data", 10);
    REQUIRE(results.size() == 2);
    const auto ctx = core.build_context("data", 10, 100);
    REQUIRE(ctx.size() == results.size());
    for (std::size_t i = 0 ; i < ctx.size() ; i++)
        REQUIRE(ctx[i].chunk_id == results[i].chunk_id);
    REQUIRE(core.search("unrelated", 10).empty());
}