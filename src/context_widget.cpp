#include "aiws/gui/context_widget.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace aiws::gui {

ContextWidget::ContextWidget(QWidget* parent) : QWidget(parent) {
    // TODO: Construct and lay out the Context area, configure the token-budget
    // control, connect local events, and establish initial state.
    QFrame* panel = new QFrame(this);
    panel->setFrameShape(QFrame::StyledPanel);
    context_budget_slider_ = new QSlider(Qt::Horizontal, panel);
    context_budget_slider_->setRange(50, 1000);
    context_budget_slider_->setSingleStep(50);
    context_budget_slider_->setPageStep(50);
    context_budget_slider_->setValue(300);
    budget_value_label_ = new QLabel("300", panel);
    build_context_button_ = new QPushButton("Build Context", panel);
    context_text_ = new QPlainTextEdit(panel);
    context_text_->setReadOnly(true);
    context_text_->setPlaceholderText("Built context will appear here.");
    QHBoxLayout* budget_row = new QHBoxLayout;
    budget_row->addWidget(new QLabel("Token budget:", panel));
    budget_row->addWidget(context_budget_slider_);
    budget_row->addWidget(budget_value_label_);
    budget_row->addWidget(build_context_button_);
    QVBoxLayout* panel_layout = new QVBoxLayout(panel);
    panel_layout->addWidget(new QLabel("<b>Context:</b>", panel));
    panel_layout->addLayout(budget_row);
    panel_layout->addWidget(context_text_);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(panel);
    connect(context_budget_slider_, &QSlider::valueChanged, this, &ContextWidget::budget_changed);
    connect(build_context_button_, &QPushButton::clicked, this, &ContextWidget::request_context);
    build_context_button_->setEnabled(false);
}

int ContextWidget::token_budget() const {
    // TODO
    return context_budget_slider_->value();
}

void ContextWidget::set_build_context_enabled(bool enabled) {
    // TODO
    build_context_button_->setEnabled(enabled);
}

void ContextWidget::set_context(const std::vector<ContextItem>& items) {
    // TODO
    QString output;
    for (const ContextItem& item : items) {
        output += "[" + QString::fromStdString(item.document_id) +
                  ", chunk " + QString::number(item.chunk_sequence) +
                  ", " + QString::number(item.token_count) + " tokens";
        if (item.truncated)
            output += ", truncated";
        output += "]\n" + QString::fromStdString(item.text) + "\n\n";
    }
    context_text_->setPlainText(output);
}

void ContextWidget::clear_context() {
    // TODO
    context_text_->clear();
}

void ContextWidget::reset() {
    // TODO
    context_budget_slider_->setValue(300);
    clear_context();
    set_build_context_enabled(false);
}

void ContextWidget::request_context() {
    // TODO
    emit build_context_requested(token_budget());
}

void ContextWidget::budget_changed(int value) {
    // TODO
    int rounded = (value + 25) / 50 * 50;
    if (rounded != value) {
        context_budget_slider_->setValue(rounded);
        return;
    }
    budget_value_label_->setText(QString::number(value));
}

}  // namespace aiws::gui
