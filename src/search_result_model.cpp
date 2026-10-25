#include "aiws/gui/search_result_model.hpp"

namespace aiws::gui {

SearchResultModel::SearchResultModel(QObject* parent)
    : QAbstractTableModel(parent) {
    // TODO
}

int SearchResultModel::rowCount(const QModelIndex& parent) const {
    // TODO
    if (parent.isValid())  return 0;
    return static_cast<int>(results_.size());
}

int SearchResultModel::columnCount(const QModelIndex& parent) const {
    // TODO
    if (parent.isValid()) return 0;
    return 5;
}

QVariant SearchResultModel::data(const QModelIndex& index, int role) const {
    // TODO
    if (role != Qt::DisplayRole)  return QVariant();
    const SearchResult* result = result_at(index.row());
    if (result == nullptr) return QVariant();
    switch (index.column()) {
        case 0: return index.row() + 1;
        case 1: return QString::fromStdString(result->document_id);
        case 2: return QString::number(result->chunk_sequence);
        case 3: return QString::number(result->score, 'f', 4);
        case 4: return QString::fromStdString(result->text).simplified();
    }
    return QVariant();
}

QVariant SearchResultModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const {
    // TODO
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();
    switch (section) {
        case 0: return "Rank";
        case 1: return "Document";
        case 2: return "Chunk";
        case 3: return "Score";
        case 4: return "Text";
    }
    return QVariant();
}

void SearchResultModel::set_results(std::vector<SearchResult> results) {
    // TODO
    beginResetModel();
    results_ = results;
    endResetModel();
}

void SearchResultModel::clear() {
    // TODO
    beginResetModel();
    results_.clear();
    endResetModel();
}

const SearchResult* SearchResultModel::result_at(int row) const noexcept {
    // TODO
    if (row < 0 || row >= static_cast<int>(results_.size())) {
        return nullptr;
    }
    return &results_[row];
}

}  // namespace aiws::gui
