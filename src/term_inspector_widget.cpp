#include "aiws/gui/term_inspector_widget.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace aiws::gui {

TermInspectorWidget::TermInspectorWidget(QWidget* parent) : QWidget(parent) {
    // TODO: Construct and lay out the Term Inspector, connect local events,
    // and establish initial state.
    QFrame* panel = new QFrame(this);
    panel->setFrameShape(QFrame::StyledPanel);
    term_edit_ = new QLineEdit(panel);
    term_edit_->setPlaceholderText("Enter a term");
    inspect_button_ = new QPushButton("Inspect", panel);
    document_frequency_value_ = new QLabel("-", panel);
    chunk_frequency_value_ = new QLabel("-", panel);
    QHBoxLayout* term_row = new QHBoxLayout;
    term_row->addWidget(new QLabel("Term:", panel));
    term_row->addWidget(term_edit_);
    term_row->addWidget(inspect_button_);
    QHBoxLayout* stats_row = new QHBoxLayout;
    stats_row->addWidget(new QLabel("Document Frequency:", panel));
    stats_row->addWidget(document_frequency_value_);
    stats_row->addStretch();
    stats_row->addWidget(new QLabel("Selected Chunk TF:", panel));
    stats_row->addWidget(chunk_frequency_value_);
    QVBoxLayout* panel_layout = new QVBoxLayout(panel);
    panel_layout->addWidget(new QLabel("<b>Term Inspector:</b>", panel));
    panel_layout->addLayout(term_row);
    panel_layout->addLayout(stats_row);
    panel_layout->addStretch();
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(panel);
    connect(inspect_button_, &QPushButton::clicked, this, &TermInspectorWidget::request_inspection);
    inspect_button_->setEnabled(false);
}

void TermInspectorWidget::set_inspection_enabled(bool enabled) {
    // TODO
    inspect_button_->setEnabled(enabled);
}

void TermInspectorWidget::set_selected_chunk(const QString& chunk_id) {
    // TODO
    selected_chunk_id_ = chunk_id;
}

void TermInspectorWidget::set_statistics(std::size_t document_frequency,
                                         bool has_chunk_frequency,
                                         std::size_t chunk_frequency) {
    // TODO
    document_frequency_value_->setText(QString::number(document_frequency));
    if (has_chunk_frequency)
        chunk_frequency_value_->setText(QString::number(chunk_frequency));
    else
        chunk_frequency_value_->setText("-");
}

void TermInspectorWidget::clear_statistics() {
    // TODO
    document_frequency_value_->setText("-");
    chunk_frequency_value_->setText("-");
}

void TermInspectorWidget::reset() {
    // TODO
    term_edit_->clear();
    selected_chunk_id_.clear();
    clear_statistics();
    set_inspection_enabled(false);
}

void TermInspectorWidget::request_inspection() {
    // TODO
    emit term_inspection_requested(term_edit_->text(), selected_chunk_id_);
}

}  // namespace aiws::gui
