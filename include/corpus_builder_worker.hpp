#pragma once

#include "aiws/processing_core.hpp"
#include "aiws/workspace.hpp"

#include <QObject>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

// m4 file creation

namespace aiws::gui {

class CorpusBuilderWorker {
public:
    using BuildFunction = std::unique_ptr<ProcessingCore> (*)(const Workspace&);

    CorpusBuilderWorker() = default;

    // calls shutdown()
    ~CorpusBuilderWorker();

    // one worker owns one thread, so copying makes no sense
    CorpusBuilderWorker(const CorpusBuilderWorker&) = delete;
    CorpusBuilderWorker& operator=(const CorpusBuilderWorker&) = delete;

    bool start(const Workspace& workspace, int delay_ms_per_document,
               QObject* receiver, const char* slot_name,
               BuildFunction builder = nullptr);

    // true from start() until finish()
    bool is_running() const noexcept;

    // stops the running build right away; it never produces a result
    void cancel() noexcept;

    // call from the slot once the build has said it's done
    void finish();

    // m5: wakes the worker, tells it to stop, and joins it (exit)
    void shutdown();

    // after finish():
    bool succeeded() const; // did the build work?
    std::string error() const; // what went wrong, if it failed?
    std::unique_ptr<ProcessingCore> take_result(); // hands over the new corpus (or nullptr)

private:
    struct Request {
        Workspace workspace;
        int delay_ms_per_document{0};
        QObject* receiver{nullptr};
        std::string slot_name;
        BuildFunction builder{nullptr};
    };

    void run();

    std::thread thread_;
    bool busy_{false};

    std::atomic<bool> cancel_requested_{false};

    mutable std::mutex mutex_;
    std::condition_variable wake_;
    Request request_;
    bool has_request_{false};
    bool stop_{false};
    std::unique_ptr<ProcessingCore> result_;
    std::string error_;
    bool succeeded_{false};
};

}  // namespace aiws::gui