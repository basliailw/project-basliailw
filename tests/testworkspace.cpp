#include <cassert>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "Document.hpp"
#include "Message.hpp"
#include "Prompt.hpp"
#include "Workspace.hpp"

void prompt_smoke_test() {
    Prompt p("Reviewer", "Review this requirement.");
    assert(p.title() == "Reviewer");
    assert(!p.empty());
}

void workspace_smoke_test() {
    Workspace w("Demo");
    w.addPrompt(Prompt("Reviewer", "Review this requirement."));
    assert(w.promptCount() == 1);
}

void document_test() {
    Document def;
    assert(def.empty());

    Document doc("Title", "Some contents.");
    assert(doc.title() == "Title");
    assert(doc.contents() == "Some contents.");
    assert(doc.sourcePath().empty());

    Document same("Title", "Some contents.");
    assert(doc == same);
    Document different("Other", "Some contents.");
    assert(doc != different);

    std::ofstream out("sample.txt");
    out << "Hello\nWorld\n";
    out.close();

    Document loaded;
    bool ok = loaded.load("sample.txt");
    assert(ok);
    assert(loaded.sourcePath() == "sample.txt");
    assert(loaded.title() == "sample.txt");
    assert(loaded.contents() == "Hello\nWorld\n");
    assert(loaded.characterCount() == loaded.contents().size());
    assert(!loaded.empty());

    bool failed = doc.load("Lakers2027.txt");
    assert(!failed);
    assert(doc.title() == "Title");
    assert(doc.contents() == "Some contents.");
}

void prompt_test() {
    Prompt def;
    assert(def.empty());

    Prompt p("Reviewer", "Review this requirement.");
    assert(p.title() == "Reviewer");
    assert(!p.empty());

    Prompt same("Reviewer", "Review this requirement.");
    assert(p == same);
    Prompt different("Reviewer", "Different text");
    assert(p != different);
}

void message_test() {
    Message def;
    assert(def.role() == MessageRole::User);
    assert(def.empty());

    Message m(MessageRole::Assistant, "Hi there.");
    assert(m.role() == MessageRole::Assistant);
    assert(m.text() == "Hi there.");

    Message same(MessageRole::Assistant, "Hi there.");
    assert(m == same);
    Message different(MessageRole::User, "Hi there.");
    assert(m != different);
}

void workspace_test() {
    Workspace w("Demo");
    w.addDocument(Document("First", "one"));
    w.addDocument(Document("Second", "two"));
    w.addPrompt(Prompt("P1", "text1"));
    assert(w.documentCount() == 2);
    assert(w.promptCount() == 1);
    assert(w.documentAt(0).title() == "First");
    assert(w.documentAt(1).title() == "Second");

    bool threw = false;
    try {
        w.documentAt(99);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    Workspace copy = w;
    w.setName("Changed");
    w.addDocument(Document("Third", "three"));
    assert(copy.name() == "Demo");
    assert(copy.documentCount() == 2);
    assert(copy != w);
}

int main() {
    prompt_smoke_test();
    workspace_smoke_test();

    // TODO: Add your own meaningful M0 tests.
    // See the Testing Requirements section of the specification.
    document_test();
    prompt_test();
    message_test();
    workspace_test();

    std::cout << "M0 tests passed\n";
    return 0;
}
