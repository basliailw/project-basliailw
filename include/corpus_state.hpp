#pragma once

#include <memory>

namespace aiws::gui {

class MainWindow;

// m5: one class per corpus state; each event returns the next state, or nullptr to stay
class CorpusState {
public:
    virtual ~CorpusState() = default;

    virtual std::unique_ptr<CorpusState> build_requested(MainWindow& window);
    virtual std::unique_ptr<CorpusState> workspace_changed(MainWindow& window);
    virtual std::unique_ptr<CorpusState> reset(MainWindow& window);
    virtual std::unique_ptr<CorpusState> build_succeeded(MainWindow& window);
    virtual std::unique_ptr<CorpusState> build_failed(MainWindow& window);

    virtual bool can_build() const;
    virtual bool corpus_is_current() const;
};

class NotBuiltState : public CorpusState {
public:
    std::unique_ptr<CorpusState> build_requested(MainWindow& window) override;
    bool can_build() const override;
};

class BuildingState : public CorpusState {
public:
    std::unique_ptr<CorpusState> workspace_changed(MainWindow& window) override;
    std::unique_ptr<CorpusState> build_succeeded(MainWindow& window) override;
    std::unique_ptr<CorpusState> build_failed(MainWindow& window) override;
};

class CancellingState : public CorpusState {
public:
    std::unique_ptr<CorpusState> build_requested(MainWindow& window) override;
    std::unique_ptr<CorpusState> build_succeeded(MainWindow& window) override;
    std::unique_ptr<CorpusState> build_failed(MainWindow& window) override;
    bool can_build() const override;
};

class RebuildQueuedState : public CorpusState {
public:
    std::unique_ptr<CorpusState> reset(MainWindow& window) override;
    std::unique_ptr<CorpusState> build_succeeded(MainWindow& window) override;
    std::unique_ptr<CorpusState> build_failed(MainWindow& window) override;
};

class CurrentState : public CorpusState {
public:
    std::unique_ptr<CorpusState> workspace_changed(MainWindow& window) override;
    bool corpus_is_current() const override;
};

}  // namespace aiws::gui