# M1 DESIGN.md

Replace this template with your own concise engineering explanation.

## 1. System structure
Describe the major responsibilities in your M1 subsystem and how they interact.

TextProcessor   - normalization and tokenzation (also records offsets and paragraph ids). single source of text semantics
Chunker         - turns a "Document" into ordered "Chunk"s
CorpusIndex     - inverted index mapping normalized term -> postings. answers document/term frequency
RetrievalEngine - normalizes a query and produces ranked "SearchResult"s via the index
ContextBuilder  - converts ranked results into budget-bounded "ContextItem"s
ProcessingCore  - owns the derived corpus state and drives "rebuild"/"search"/"build_context". validates public arguments

## 2. Design decisions
Explain the principal data structures, interfaces, and ownership decisions in your implementation and why you selected them.

- One normalizer for docs & queries -> terms match at index and query time
- "TokenInfo" holds original offsets & paragraph id -> spans and boundary choice in one pass
- Index stores integer indices, not pointers -> safe across move/rebuild
- "Chunk" carries "document_order" & "sequence" -> deterministic tie-breaks without lookups
- Term validation at the public boundary (0 tokens -> 0, >1 -> throw). internal APIs take normalized temrs, "noexcept"

## 3. Correctness and consistency
Identify the important invariants or failure cases your design must preserve and explain how your design addresses them.

- Same normalization path for index and query
- Rebuild atomic: validate duplicate ids first, build into locals, commit by move -> failed rebuild keeps prior corpus
- Scores rounded to 12 digits. Ties by "document_order" then "sequence"
- Full replace each rebuild -> no stale or duplicate state
- Edge cases:
    - empty/punctuation -> no chunks
    - unknown terms scoreless
    - k<0 throws
    - k==0 empty

## 4. Testing strategy
Explain what your tests cover and which risks or boundaries you considered most important.

- Catch2 "TEST_CASE"/"SECTION"
- Covers:
    - normalization edges
    - offsets
    - tie determinism
    - multi-doc flow
    - etc.

## 5. Alternatives considered
Discuss at least two plausible design alternatives and why you did not choose them.

Index holding pointers to chunks:
    - Convenient, but dangles on rebuild/move
    - Integer indices into owned vector
Rescan per query, no index:
    - Violates "retrieve through the index"
    - Scales poorly
    - Inverted index