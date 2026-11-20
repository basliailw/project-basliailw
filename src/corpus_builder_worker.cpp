#include "corpus_builder_worker.hpp"

#include <QMetaObject>

#include <chrono>
#include <exception>
#include <utility>

namespace aiws::gui {

namespace {

std::unique_ptr<ProcessingCore> default_build(const Workspace& workspace) {
    std::unique_ptr<ProcessingCore> core = std::make_unique<ProcessingCore>();
    core->rebuild(workspace);
    return core;
}
}  // namespace

CorpusBuilderWorker::~CorpusBuilderWorker() {
    shutdown();
}

bool CorpusBuilderWorker::start(const Workspace& workspace, int delay_ms_per_document,
                                QObject* receiver, const char* slot_name, BuildFunction builder) {
    if (busy_)
        return false;

    if (builder == nullptr)
        builder = &default_build;

    // m5: the one worker thread is created on the first build and reused after that
    if (!thread_.joinable())
        thread_ = std::thread(&CorpusBuilderWorker::run, this);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        request_ = Request{workspace, delay_ms_per_document, receiver, std::string(slot_name), builder};
        has_request_ = true;
        cancel_requested_ = false;
    }
    wake_.notify_one();
    busy_ = true;
    return true;
}

bool CorpusBuilderWorker::is_running() const noexcept {
    return busy_;
}

void CorpusBuilderWorker::cancel() noexcept {
#ifdef AIWS_FAULT_NO_CANCEL
    return; // fault: the useless build runs to the end
#endif
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cancel_requested_ = true;
    }
    wake_.notify_one();
}

void CorpusBuilderWorker::finish() {
    busy_ = false;
}

void CorpusBuilderWorker::shutdown() {
    if (!thread_.joinable())
        return;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
        cancel_requested_ = true;
    }
#ifndef AIWS_FAULT_SILENT_SHUTDOWN
    wake_.notify_one(); // fault: without this, an idle worker never wakes and exit hangs
#endif
    thread_.join();
}

bool CorpusBuilderWorker::succeeded() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return succeeded_;
}

std::string CorpusBuilderWorker::error() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return error_;
}

std::unique_ptr<ProcessingCore> CorpusBuilderWorker::take_result() {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::move(result_);
}

void CorpusBuilderWorker::run() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (true) {
        // m5: sleep on the condition variable until there's a request or it's time to stop
        while (!has_request_ && !stop_)
            wake_.wait(lock);
        if (stop_)
            return;

        Request request = std::move(request_);
        has_request_ = false;

        // m5: each document's build time is a wait that cancel() can cut short
        const std::size_t document_count = request.workspace.documents().size();
        for (std::size_t i = 0; i < document_count && !cancel_requested_; ++i) {
            auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(request.delay_ms_per_document);
            while (!cancel_requested_ && wake_.wait_until(lock, until) == std::cv_status::no_timeout) {
            }
        }
        bool cancelled = cancel_requested_;
        lock.unlock();

        std::unique_ptr<ProcessingCore> core;
        std::string error;
        if (!cancelled) {
            try {
                core = request.builder(request.workspace);
            } catch (const std::exception& e) {
                error = e.what();
            } catch (...) {
                error = "unknown error";
            }
        }

        lock.lock();
        if (stop_)
            return;
        succeeded_ = (core != nullptr);
        result_ = std::move(core);
        error_ = error;

        // tell gui we done. run the slot -> removes block
        QMetaObject::invokeMethod(request.receiver, request.slot_name.c_str(), Qt::QueuedConnection);
    }
}

}  // namespace aiws::gui