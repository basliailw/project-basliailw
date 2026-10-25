#include "aiws/gui/workspace_widget.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace aiws::gui {

WorkspaceWidget::WorkspaceWidget(QWidget* parent) : QWidget(parent) {
    // TODO: Construct and lay out the Workspace and Selected Document areas,
    // connect local widget events, and establish initial presentation state.
    QFrame* left = new QFrame(this);
    left->setFrameShape(QFrame::StyledPanel);
    QLabel* workspace_label = new QLabel("<b>Workspace</b>", left);
    add_document_button_ = new QPushButton("Add Document", left);
    build_corpus_button_ = new QPushButton("Build Corpus", left);
    document_list_ = new QListWidget(left);
    corpus_status_label_ = new QLabel("Not built", left);
    corpus_status_label_->setStyleSheet("font-weight: bold;");
    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addWidget(workspace_label);
    buttons->addWidget(add_document_button_);
    buttons->addWidget(build_corpus_button_);
    buttons->addStretch();
    QHBoxLayout* corpus_row = new QHBoxLayout;
    corpus_row->addWidget(new QLabel("Corpus:", left));
    corpus_row->addWidget(corpus_status_label_);
    corpus_row->addStretch();
    QVBoxLayout* left_layout = new QVBoxLayout(left);
    left_layout->addLayout(buttons);
    left_layout->addWidget(document_list_);
    left_layout->addLayout(corpus_row);
    QFrame* right = new QFrame(this);
    right->setFrameShape(QFrame::StyledPanel);
    document_title_label_ = new QLabel("Selected Document -", right);
    document_title_label_->setStyleSheet("font-weight: bold;");
    document_text_ = new QPlainTextEdit(right);
    document_text_->setReadOnly(true);
    document_text_->setPlaceholderText("Select a document to view its contents.");
    QVBoxLayout* right_layout = new QVBoxLayout(right);
    right_layout->addWidget(document_title_label_);
    right_layout->addWidget(document_text_);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(left);
    layout->addWidget(right);
    connect(add_document_button_,     &QPushButton::clicked, this, &WorkspaceWidget::add_document_requested);
    connect(build_corpus_button_,     &QPushButton::clicked, this, &WorkspaceWidget::build_corpus_requested);
    connect(document_list_, &QListWidget::currentRowChanged, this,      &WorkspaceWidget::document_selected);
    build_corpus_button_->setEnabled(false);
}

void WorkspaceWidget::add_document(const QString& title) {
    // TODO
    document_list_->addItem(title);
}

void WorkspaceWidget::select_document(int row) {
    // TODO
    document_list_->setCurrentRow(row);
}

void WorkspaceWidget::show_selected_document(const QString& title,
                                              const QString& text) {
    // TODO
    document_title_label_->setText("Selected Document - " + title);
    document_text_->setPlainText(text);
}

void WorkspaceWidget::clear_selected_document() {
    // TODO
    document_title_label_->setText("Selected Document -");
    document_text_->clear();
}

void WorkspaceWidget::set_corpus_status(const QString& text) {
    // TODO
    (void)text;  // To suppress unused-parameter warning.
    corpus_status_label_->setText(text);
}

void WorkspaceWidget::set_build_corpus_enabled(bool enabled) {
    // TODO
    build_corpus_button_->setEnabled(enabled);
}

void WorkspaceWidget::reset() {
    // TODO
    document_list_->clear();
    clear_selected_document();
    set_corpus_status("Not built");
    set_build_corpus_enabled(false);
}

}  // namespace aiws::gui
