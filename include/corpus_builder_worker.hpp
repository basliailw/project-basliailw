#pragma once

#include "aiws/processing_core.hpp"
#include "aiws/workspace.hpp"

#include <QObject>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

// m4 file creation

namespace aiws::gui {

class CorpusBuilderWorker {
public:
    using BuildFunction = std::unique_ptr<ProcessingCore> (*)(const Workspace&);

    CorpusBuilderWorker() = default;

    // tells any running build to stop, then waits for its thread to end
    ~CorpusBuilderWorker();

    // one worker owns one thread, so copying makes no sense
    CorpusBuilderWorker(const CorpusBuilderWorker&) = delete;
    CorpusBuilderWorker& operator=(const CorpusBuilderWorker&) = delete;

    bool start(const Workspace& workspace, int delay_ms_per_document,
               QObject* receiver, const char* slot_name,
               BuildFunction builder = nullptr);

    // true from start() until finish() has joined the thread
    bool is_running() const noexcept;

    // cancel()
    void cancel() noexcept;

    // only call this once the thread has said it's done (from the slot), or on shutdown right after cancel()
    void finish();

    // after finish():
    // bool was_cancelled() const noexcept; // did we stop early?
    bool succeeded() const noexcept; // did the build work?
    const std::string& error() const noexcept; // what went wrong, if it failed?
    std::unique_ptr<ProcessingCore> take_result(); // hands over the new corpus (or nullptr)

private:
    // the actual build thread.
    void run(Workspace snapshot, int delay_ms_per_document,
             QObject* receiver, std::string slot_name, BuildFunction builder);

    std::thread thread_;

    std::atomic<bool> cancel_requested_{false};

    std::unique_ptr<ProcessingCore> result_;
    std::string error_;
    bool succeeded_{false};
    // bool cancelled_{false};
};

}  // namespace aiws::gui