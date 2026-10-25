#include "aiws/gui/main_window.hpp"
#include "aiws/gui/context_widget.hpp"
#include "aiws/gui/resettable.hpp"
#include "aiws/gui/search_widget.hpp"
#include "aiws/gui/term_inspector_widget.hpp"
#include "aiws/gui/workspace_widget.hpp"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace aiws::gui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // TODO: Construct the M3 application window, compose the GUI components,
    // establish layouts/presentation, connect events, and establish initial state.
    setWindowTitle("AI Workspace");
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QLabel* header = new QLabel("<b><font size=6>AI Workspace</font></b><br>" "<i>Document Search and Context Explorer</i>", central);
    header->setStyleSheet("background-color: #6b1f2a; color: white; padding: 10px;");
    workspace_widget_ = new WorkspaceWidget(central);
    search_widget_ = new SearchWidget(central);
    term_inspector_widget_ = new TermInspectorWidget(central);
    context_widget_ = new ContextWidget(central);
    status_label_ = new QLabel("Current status: Ready", central);
    reset_button_ = new QPushButton("Reset", central);
    exit_button_ = new QPushButton("Exit", central);
    reset_button_->setStyleSheet("background-color: #d9a441; font-weight: bold; padding: 5px 30px;");
    exit_button_->setStyleSheet("background-color: #3c6db4; color: white; font-weight: bold; padding: 5px 30px;");
    QHBoxLayout* bottom_row = new QHBoxLayout;
    bottom_row->addWidget(term_inspector_widget_);
    bottom_row->addWidget(context_widget_);
    QHBoxLayout* status_row = new QHBoxLayout;
    status_row->addWidget(status_label_);
    status_row->addStretch();
    status_row->addWidget(reset_button_);
    status_row->addWidget(exit_button_);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->addWidget(header);
    layout->addWidget(workspace_widget_, 3);
    layout->addWidget(search_widget_, 3);
    layout->addLayout(bottom_row, 2);
    layout->addLayout(status_row);
    connect(workspace_widget_, &WorkspaceWidget::add_document_requested,
            this, &MainWindow::add_document);
    connect(workspace_widget_, &WorkspaceWidget::build_corpus_requested,
            this, &MainWindow::build_corpus);
    connect(workspace_widget_, &WorkspaceWidget::document_selected,
            this, &MainWindow::document_selected);
    connect(search_widget_, &SearchWidget::search_requested,
            this, &MainWindow::search_requested);
    connect(search_widget_, &SearchWidget::query_changed,
            this, &MainWindow::query_changed);
    connect(search_widget_, &SearchWidget::source_document_requested,
            this, &MainWindow::source_document_requested);
    connect(search_widget_, &SearchWidget::selected_chunk_changed,
            term_inspector_widget_, &TermInspectorWidget::set_selected_chunk);
    connect(term_inspector_widget_, &TermInspectorWidget::term_inspection_requested,
            this, &MainWindow::term_inspection_requested);
    connect(context_widget_, &ContextWidget::build_context_requested,
            this, &MainWindow::build_context_requested);
    connect(reset_button_, &QPushButton::clicked, this, &MainWindow::reset_application);
    connect(exit_button_, &QPushButton::clicked, this, &MainWindow::exit_application);
    resize(900, 900);
    update_action_states();
}

void MainWindow::set_exit_callback(ExitCallback callback) noexcept {
    // TODO
    exit_callback_ = callback;
}

void MainWindow::add_document() {
    // TODO
    QString path = QFileDialog::getOpenFileName(this, "Add Document", "",
                                                "Text files (*.txt);;All files (*)");
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        status_label_->setText("Current status: Could not open file.");
        return;
    }
    QString text = QString::fromUtf8(file.readAll());
    QFileInfo info(path);
    QString id = info.completeBaseName();
    QString title = info.fileName();
    workspace_.add_document(Document(id.toStdString(), title.toStdString(), text.toStdString()));
    invalidate_corpus();
    workspace_widget_->add_document(title);
    workspace_widget_->select_document(static_cast<int>(workspace_.documents().size()) - 1);
    status_label_->setText("Current status: Added " + title);
}

void MainWindow::build_corpus() {
    // TODO
    processing_core_.rebuild(workspace_);
    corpus_current_ = true;
    clear_processing_outputs();
    workspace_widget_->set_corpus_status("Built (" + QString::number(processing_core_.chunk_count()) + " chunks)");
    status_label_->setText("Current status: Corpus built.");
    update_action_states();
}

void MainWindow::document_selected(int row) {
    // TODO
    if (row < 0 || row >= static_cast<int>(workspace_.documents().size())) {
        workspace_widget_->clear_selected_document();
        return;
    }
    const Document& doc = workspace_.documents()[row];
    workspace_widget_->show_selected_document(QString::fromStdString(doc.title()), QString::fromStdString(doc.text()));
}

void MainWindow::search_requested(const QString& query, int k) {
    // TODO
    if (!corpus_current_) return;
    std::vector<SearchResult> results = processing_core_.search(query.toStdString(), k);
    int count = static_cast<int>(results.size());
    term_inspector_widget_->clear_statistics();
    context_widget_->clear_context();
    search_widget_->set_results(results);
    status_label_->setText("Current status: " + QString::number(count) + " result(s) found.");
}

void MainWindow::query_changed(const QString& query) {
    // TODO
    (void)query;  // To suppress unused-parameter warning.
    update_action_states();
}

void MainWindow::build_context_requested(int token_budget) {
    // TODO
    if (!corpus_current_) return;
    std::vector<ContextItem> items = processing_core_.build_context(search_widget_->query().toStdString(), search_widget_->result_count(), static_cast<std::size_t>(token_budget));
    context_widget_->set_context(items);
    status_label_->setText("Current status: Context built.");
}

void MainWindow::source_document_requested(const QString& document_id) {
    // TODO
    const std::vector<Document>& docs = workspace_.documents();
    for (int i = 0; i < static_cast<int>(docs.size()); ++i)
        if (docs[i].id() == document_id.toStdString()) {
            workspace_widget_->select_document(i);
            status_label_->setText("Current status: Showing source document.");
            return;
        }
}

void MainWindow::term_inspection_requested(const QString& term,
                                           const QString& chunk_id) {
    // TODO
    if (!corpus_current_ || term.trimmed().isEmpty()) return;
    std::string t = ProcessingCore::normalize(term.trimmed().toStdString());
    std::size_t df = processing_core_.document_frequency(t);
    bool has_chunk = !chunk_id.isEmpty();
    std::size_t tf = 0;
    if (has_chunk) tf = processing_core_.term_frequency(t, chunk_id.toStdString());
    term_inspector_widget_->set_statistics(df, has_chunk, tf);
    status_label_->setText("Current status: Inspected term " + term.trimmed());
}

void MainWindow::reset_application() {
    // TODO
    workspace_.clear();
    corpus_current_ = false;
    Resettable* parts[] = {workspace_widget_, search_widget_, term_inspector_widget_, context_widget_};
    for (Resettable* part : parts) part->reset();
    status_label_->setText("Current status: Application reset.");
    update_action_states();
}
 
void MainWindow::exit_application() {
    // TODO
    if (exit_callback_ != nullptr) exit_callback_(this);
}

void MainWindow::update_action_states() {
    // TODO
    bool has_documents = !workspace_.documents().empty();
    bool has_query = search_widget_->has_query();
    workspace_widget_->set_build_corpus_enabled(has_documents && !corpus_current_);
    search_widget_->set_search_enabled(corpus_current_ && has_query);
    term_inspector_widget_->set_inspection_enabled(corpus_current_);
    context_widget_->set_build_context_enabled(corpus_current_ && has_query);
}

void MainWindow::invalidate_corpus() {
    // TODO
    corpus_current_ = false;
    workspace_widget_->set_corpus_status("Not built");
    clear_processing_outputs();
    update_action_states();
}

void MainWindow::clear_processing_outputs() {
    // TODO
    search_widget_->clear_results();
    term_inspector_widget_->clear_statistics();
    context_widget_->clear_context();
}

}  // namespace aiws::gui
