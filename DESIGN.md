# M2 Technical & Design Understanding

This file is an assessed technical-understanding artifact, not ordinary project documentation.
Answer all four questions using your own submitted implementation. Concise answers are acceptable when they are technically correct and specific.

Generic descriptions of C++ concepts or restatements of the assignment that do not identify and explain corresponding parts of your code will receive limited credit.

## 1. Polymorphism and dynamic dispatch - 1.5 points

ProcessingCore::search calls search on its retrieval member, which is a unique_ptr to the RetrievalStrategy interface. With the default constructor the actual object is a RetrievalEngine. Since search is declared pure virtual in RetrievalStrategy and overridden in RetrievalEngine, the call is resolved at run time based on the real object, so RetrievalEngine's version runs. An injected strategy is also reached the same way. If search weren't virtual, the call would be fixed at compile time to the base class version, and injected strategies would be ignored.

## 2. Ownership and lifetime - 1.5 points

The default constructor constructs a RetrievalEngine with make_unique and passes it to the three-argument constructor. That constructor rejects null pointers with invalid_argument, then moves each pointer into ProcessingCore's Impl, which becomes the sole owner. The strategy is destroyed automatically when the ProcessingCore is destroyed. ProcessingCore is move-only because a unique_ptr can't be copied, a copy would mean two owners deleting the same strategy. The strategy interfaces require virtual destructors because ProcessingCore deletes the concrete objects through base-class pointers, otherwise the derived destructor wouldn't run.

## 3. Architecture, extensibility, and M1 compatibility - 1.5 points

Impl now holds one unique_ptr per strategy interface instead of concrete Chunker, RetrievalEngine and ContextBuilder members, and rebuild, search and build_context only call the interface methods. The default constructor installs Chunker, RetrievalEngine and ContextBuilder with their unchanged M1 algorithms, so M1 behavior is preserved; my M1 tests still pass. A different implementation is substituted by passing it to the three-argument constructor, without changing the ProcessingCore API. An alternative would be an enum plus a switch inside ProcessingCore. That requires editing ProcessingCore for every new algorithm and can't accept outside classes, so the interface design is better.

## 4. Testing and defect reasoning - 1.5 points

Test: "M2: injected strategy is used instead of the default." It uses MarkerChunking, which produces one chunk with the text "marker" per document, and checks that the core's chunks and search results come from it, and that searching for a word from the original text finds nothing. This validates that an injected chunking strategy is actually used by rebuild. It would catch rebuild still calling a hard-coded Chunker, or chunk not being virtual. The public tests only use the default constructor, so they pass even if the injected strategy is never called. Because "marker" never appears in the documents, this test only passes if runtime substitution really happens.