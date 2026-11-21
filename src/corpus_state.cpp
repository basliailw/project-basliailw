#include "corpus_state.hpp"

#include "aiws/gui/main_window.hpp"

namespace aiws::gui {

// by default an event changes nothing
std::unique_ptr<CorpusState> CorpusState::build_requested(MainWindow&) { return nullptr; }
std::unique_ptr<CorpusState> CorpusState::workspace_changed(MainWindow&) { return nullptr; }
std::unique_ptr<CorpusState> CorpusState::reset(MainWindow& window) { return workspace_changed(window); }
std::unique_ptr<CorpusState> CorpusState::build_succeeded(MainWindow&) { return nullptr; }
std::unique_ptr<CorpusState> CorpusState::build_failed(MainWindow&) { return nullptr; }
bool CorpusState::can_build() const { return false; }
bool CorpusState::corpus_is_current() const { return false; }

std::unique_ptr<CorpusState> NotBuiltState::build_requested(MainWindow& window) {
    window.start_build();
    return std::make_unique<BuildingState>();
}

bool NotBuiltState::can_build() const { return true; }

std::unique_ptr<CorpusState> BuildingState::workspace_changed(MainWindow& window) {
    window.cancel_build();
    return std::make_unique<CancellingState>();
}

std::unique_ptr<CorpusState> BuildingState::build_succeeded(MainWindow& window) {
    window.install_result();
    return std::make_unique<CurrentState>();
}

std::unique_ptr<CorpusState> BuildingState::build_failed(MainWindow& window) {
    window.show_build_failed();
    return std::make_unique<NotBuiltState>();
}

// the build that finishes here is no longer wanted, so its result is dropped
std::unique_ptr<CorpusState> CancellingState::build_requested(MainWindow&) { return std::make_unique<RebuildQueuedState>(); }
std::unique_ptr<CorpusState> CancellingState::build_succeeded(MainWindow&) { return std::make_unique<NotBuiltState>(); }
std::unique_ptr<CorpusState> CancellingState::build_failed(MainWindow&) { return std::make_unique<NotBuiltState>(); }
bool CancellingState::can_build() const { return true; }

// a rebuild waits only for the cancelled build to report back, then starts on the latest workspace
std::unique_ptr<CorpusState> RebuildQueuedState::reset(MainWindow&) { return std::make_unique<CancellingState>(); }

std::unique_ptr<CorpusState> RebuildQueuedState::build_succeeded(MainWindow& window) {
    window.start_build();
    return std::make_unique<BuildingState>();
}

std::unique_ptr<CorpusState> RebuildQueuedState::build_failed(MainWindow& window) {
    window.start_build();
    return std::make_unique<BuildingState>();
}

std::unique_ptr<CorpusState> CurrentState::workspace_changed(MainWindow&) { return std::make_unique<NotBuiltState>(); }

bool CurrentState::corpus_is_current() const { return true; }

}  // namespace aiws::gui