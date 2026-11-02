// M4 tests for the background build (R1-R4).

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "aiws/gui/main_window.hpp"
#include "corpus_builder_worker.hpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QTemporaryDir>

#include <stdexcept>

using namespace aiws;
using namespace aiws::gui;

// Counts how many times its slot gets called (stands in for the GUI in the R3 test).
class Counter : public QObject {
    Q_OBJECT
public slots:
    void hit() { ++calls; }
public:
    int calls = 0;
};

static QString make_file(const QTemporaryDir& dir, const QString& name) {
    QFile file(dir.filePath(name));
    file.open(QIODevice::WriteOnly);
    file.write("alpha beta gamma");
    return file.fileName();
}

static QPushButton* button(MainWindow& w, const QString& text) {
    for (QPushButton* b : w.findChildren<QPushButton*>())
        if (b->text() == text) return b;
    return nullptr;
}

static bool is_built(MainWindow& w) {
    for (QLabel* l : w.findChildren<QLabel*>())
        if (l->text().startsWith("Built (")) return true;
    return false;
}

// A build that always fails, for the R3 test.
static std::unique_ptr<ProcessingCore> failing_build(const Workspace&) {
    throw std::runtime_error("boom");
}

// Keeps the event loop running until the build ends (accepted, or Build is clickable again).
static void wait_for_build(MainWindow& w) {
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 5000 && !is_built(w) && !button(w, "Build Corpus")->isEnabled())
        QApplication::processEvents();
}

TEST_CASE("R1: Build Corpus doesn't freeze the GUI") {
    QTemporaryDir dir;
    MainWindow w;
    w.set_build_delay_ms(300);
    w.add_document_file(make_file(dir, "a.txt"));

    QElapsedTimer t;
    t.start();
    button(w, "Build Corpus")->click();
    CHECK(t.elapsed() < 100);  // returned right away instead of waiting 300 ms

    wait_for_build(w);
    CHECK(is_built(w));
}

TEST_CASE("R2: a document added mid-build makes that build stale") {
    QTemporaryDir dir;
    MainWindow w;
    w.set_build_delay_ms(300);
    w.add_document_file(make_file(dir, "a.txt"));
    button(w, "Build Corpus")->click();
    w.add_document_file(make_file(dir, "b.txt"));  // still building, so it's now stale

    wait_for_build(w);
    CHECK_FALSE(is_built(w));
}

TEST_CASE("R3: a failed build is reported back") {
    Workspace workspace;
    workspace.add_document(Document("a", "a.txt", "alpha"));
    CorpusBuilderWorker worker;
    Counter gui;

    worker.start(workspace, 0, &gui, "hit", &failing_build);

    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 3000 && gui.calls == 0) QApplication::processEvents();

    REQUIRE(gui.calls == 1);  // the failure made it back
    worker.finish();
    CHECK_FALSE(worker.succeeded());
    CHECK(worker.error() == "boom");
}

TEST_CASE("R4: closing mid-build doesn't hang") {
    QTemporaryDir dir;
    QElapsedTimer t;
    {
        MainWindow w;
        w.set_build_delay_ms(10000);
        w.add_document_file(make_file(dir, "a.txt"));
        button(w, "Build Corpus")->click();
        t.start();
    }  // window closes here mid-build
    CHECK(t.elapsed() < 1000);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    return Catch::Session().run(argc, argv);
}

#include "student_tests.moc"