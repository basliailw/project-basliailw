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
    cancel();
    finish();
}

bool CorpusBuilderWorker::start(const Workspace& workspace, int delay_ms_per_document,
                                QObject* receiver, const char* slot_name, BuildFunction builder) {
    if (thread_.joinable())
        return false;

    if (builder == nullptr)
        builder = &default_build;

    // clear out whatever the last build left behind.
    cancel_requested_ = false;
    result_.reset();
    error_.clear();
    succeeded_ = false;
    cancelled_ = false;

    // thread gets its own copies of the workspace and slot name
    thread_ = std::thread(&CorpusBuilderWorker::run, this, workspace,
                          delay_ms_per_document, receiver, std::string(slot_name),
                          builder);
    return true;
}

bool CorpusBuilderWorker::is_running() const noexcept {
    return thread_.joinable();
}

void CorpusBuilderWorker::cancel() noexcept {
    cancel_requested_ = true;
}

void CorpusBuilderWorker::finish() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

bool CorpusBuilderWorker::succeeded() const noexcept {
    return succeeded_;
}

const std::string& CorpusBuilderWorker::error() const noexcept {
    return error_;
}

std::unique_ptr<ProcessingCore> CorpusBuilderWorker::take_result() {
    return std::move(result_);
}

void CorpusBuilderWorker::run(Workspace snapshot, int delay_ms_per_document,
                              QObject* receiver, std::string slot_name,
                              BuildFunction builder) {
    // 10 ms chunks so a cancel (like hitting Exit) kicks in fast
    const std::size_t document_count = snapshot.documents().size();
    for (std::size_t i = 0; i < document_count && !cancel_requested_; ++i) {
        for (int waited = 0; waited < delay_ms_per_document && !cancel_requested_; waited += 10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    if (cancel_requested_) {
        cancelled_ = true;  // when prompted to stop, skips the real build.
    } else {
        // If anything throws, save the message for the GUI.
        try {
            result_ = builder(snapshot);
            succeeded_ = true;
        } catch (const std::exception& e) {
            error_ = e.what();
        } catch (...) {
            error_ = "Unknown error";
        }
    }

#ifdef AIWS_FAULT_LOSE_FAILURE // m4: fault switch (R3)
    if (!succeeded_) {
        return;
#endif

    // tell gui we done. run the slot -> removes block
    QMetaObject::invokeMethod(receiver, slot_name.c_str(), Qt::QueuedConnection);
}

}  // namespace aiws::gui