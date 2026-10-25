#include "aiws/gui/search_widget.hpp"
#include "aiws/gui/search_result_model.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QVBoxLayout>

namespace aiws::gui {

SearchWidget::SearchWidget(QWidget* parent) : QWidget(parent) {
    // TODO: Construct and lay out the Query, Search Results, and Selected Result
    // areas; configure the table/model; connect local events; establish initial state.
    QFrame* panel = new QFrame(this);
    panel->setFrameShape(QFrame::StyledPanel);
    query_edit_ = new QLineEdit(panel);
    query_edit_->setPlaceholderText("Search the current corpus");
    result_count_spin_ = new QSpinBox(panel);
    result_count_spin_->setRange(1, 100);
    result_count_spin_->setValue(3);
    search_button_ = new QPushButton("Search", panel);
    result_model_ = new SearchResultModel(this);
    results_view_ = new QTableView(panel);
    results_view_->setModel(result_model_);
    results_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    results_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    results_view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    results_view_->verticalHeader()->setVisible(false);
    results_view_->horizontalHeader()->setStretchLastSection(true);
    view_source_button_ = new QPushButton("View Source", panel);
    selected_result_text_ = new QPlainTextEdit(panel);
    selected_result_text_->setReadOnly(true);
    selected_result_text_->setPlaceholderText("Select a result to inspect its details.");
    QHBoxLayout* query_row = new QHBoxLayout;
    query_row->addWidget(new QLabel("<b>Query:</b>", panel));
    query_row->addWidget(query_edit_);
    query_row->addWidget(new QLabel("Results:", panel));
    query_row->addWidget(result_count_spin_);
    query_row->addWidget(search_button_);
    QHBoxLayout* selected_row = new QHBoxLayout;
    selected_row->addWidget(new QLabel("<b>Selected Result:</b>", panel));
    selected_row->addStretch();
    selected_row->addWidget(view_source_button_);
    QVBoxLayout* panel_layout = new QVBoxLayout(panel);
    panel_layout->addLayout(query_row);
    panel_layout->addWidget(results_view_, 3);
    panel_layout->addLayout(selected_row);
    panel_layout->addWidget(selected_result_text_, 2);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(panel);
    connect(search_button_, &QPushButton::clicked, this, &SearchWidget::request_search);
    connect(query_edit_, &QLineEdit::textChanged, this, &SearchWidget::query_changed);
    connect(results_view_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &SearchWidget::show_selected_result);
    connect(view_source_button_, &QPushButton::clicked, this, &SearchWidget::request_source_document);
    search_button_->setEnabled(false);
    view_source_button_->setEnabled(false);
}

QString SearchWidget::query() const {
    // TODO
    return query_edit_->text();
}

int SearchWidget::result_count() const {
    // TODO
     return result_count_spin_->value();
}

bool SearchWidget::has_query() const {
    // TODO
    return !query_edit_->text().trimmed().isEmpty();
}

void SearchWidget::set_search_enabled(bool enabled) {
    // TODO
    search_button_->setEnabled(enabled);
}

void SearchWidget::set_results(std::vector<SearchResult> results) {
    // TODO
    clear_results();
    result_model_->set_results(results);
}

void SearchWidget::clear_results() {
    // TODO
    result_model_->clear();
    selected_result_text_->clear();
    view_source_button_->setEnabled(false);
    emit selected_chunk_changed("");
}

void SearchWidget::reset() {
    // TODO
    query_edit_->clear();
    result_count_spin_->setValue(3);
    clear_results();
    set_search_enabled(false);
}

void SearchWidget::request_search() {
    // TODO
    emit search_requested(query(), result_count());
}

void SearchWidget::show_selected_result() {
    // TODO
    int row = results_view_->currentIndex().row();
    const SearchResult* result = result_model_->result_at(row);
    if (result == nullptr) {
        selected_result_text_->clear();
        view_source_button_->setEnabled(false);
        emit selected_chunk_changed("");
        return;
    }
    QString details = "Document: " + QString::fromStdString(result->document_id) +
                      "\nChunk: " + QString::number(result->chunk_sequence) +
                      "\nScore: " + QString::number(result->score) +
                      "\n\n" + QString::fromStdString(result->text);
    selected_result_text_->setPlainText(details);
    view_source_button_->setEnabled(true);
    emit selected_chunk_changed(QString::fromStdString(result->chunk_id));
}

void SearchWidget::request_source_document() {
    // TODO
    const SearchResult* result = result_model_->result_at(results_view_->currentIndex().row());
    if (result != nullptr)
        emit source_document_requested(QString::fromStdString(result->document_id));
}

}  // namespace aiws::gui
