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
#include <QFileInfo>
#include <QListWidget>
#include <QProcess>
#include <QThread>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace aiws;
using namespace aiws::gui;

// Counts how many times its slot gets called (stands in for the GUI in the R3 test)
class Counter : public QObject {
    Q_OBJECT
public slots:
    void hit() { calls++; }
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

// A build that always fails, for the R3 test
static std::unique_ptr<ProcessingCore> failing_build(const Workspace&) {
    throw std::runtime_error("boom");
}

// Keeps the event loop running until the build ends (accepted, or Build is clickable again)
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

static int run_aiws_send(const QString& dir, const QString& socket_path, const QString& file) {
    QProcess process;
    process.setWorkingDirectory(dir);
    process.start(AIWS_SEND_PATH, {"--socket", socket_path, "add", file});
    process.waitForFinished(10000);
    return process.exitCode();
}

TEST_CASE("E1+E2: aiws_send adds a document from an odd relative path") {
    QTemporaryDir dir;
    QString name = QString::fromUtf8("my file \xC3\xA9\nnew.txt"); // space, é, newline
    make_file(dir, name);
    MainWindow w;
    REQUIRE(w.start_ipc_server(dir.filePath("s.sock")));

    CHECK(run_aiws_send(dir.path(), dir.filePath("s.sock"), name) == 0);
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 200) QApplication::processEvents(); // let the add reach the window
    CHECK(w.findChild<QListWidget*>()->count() == 1);
}

TEST_CASE("E3: a request split into pieces still works") {
    QTemporaryDir dir;
    MainWindow w;
    REQUIRE(w.start_ipc_server(dir.filePath("s.sock")));

    // 'A', 4-byte big-endian length (path is under 256 bytes), path
    QByteArray path = make_file(dir, "a.txt").toUtf8();
    QByteArray request = QByteArray("A") + QByteArray(3, '\0') + char(path.size()) + path;

    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::strcpy(address.sun_path, dir.filePath("s.sock").toUtf8().constData());
    REQUIRE(::connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);

    ::write(fd, request.constData(), 3); // first piece
    QThread::msleep(200); // pause
    ::write(fd, request.constData() + 3, request.size() - 3); // the rest

    char status = 9;
    ::read(fd, &status, 1);
    ::close(fd);
    CHECK(status == 0); // accepted
}

TEST_CASE("E4: closing the app stops serving fast and removes the socket") {
    QTemporaryDir dir;
    QElapsedTimer t;
    {
        MainWindow w;
        REQUIRE(w.start_ipc_server(dir.filePath("s.sock")));
        t.start();
    }  // window closes here
    CHECK(t.elapsed() < 500);
    CHECK_FALSE(QFileInfo::exists(dir.filePath("s.sock")));
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    return Catch::Session().run(argc, argv);
}

#include "student_tests.moc"