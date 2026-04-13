#include "log/log_filter_proxy_model.h"

#include "log/log_model.h"

LogFilterProxyModel::LogFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

LogModel *LogFilterProxyModel::sourceLogModel() const
{
    return m_sourceLogModel;
}

void LogFilterProxyModel::setSourceLogModel(LogModel *model)
{
    if (m_sourceLogModel == model)
        return;

    m_sourceLogModel = model;
    setSourceModel(model);

    emit sourceLogModelChanged();
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

QString LogFilterProxyModel::levelFilter() const
{
    return m_levelFilter;
}

void LogFilterProxyModel::setLevelFilter(const QString &level)
{
    if (m_levelFilter == level)
        return;

    m_levelFilter = level;

    emit levelFilterChanged();
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

QString LogFilterProxyModel::searchText() const
{
    return m_searchText;
}

void LogFilterProxyModel::setSearchText(const QString &text)
{
    if (m_searchText == text)
        return;

    m_searchText = text;

    emit searchTextChanged();
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

bool LogFilterProxyModel::showOnlyUnacknowledged() const
{
    return m_showOnlyUnacknowledged;
}

void LogFilterProxyModel::setShowOnlyUnacknowledged(bool value)
{
    if (m_showOnlyUnacknowledged == value)
        return;

    m_showOnlyUnacknowledged = value;

    emit showOnlyUnacknowledgedChanged();
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

void LogFilterProxyModel::setAcknowledged(int proxyRow, bool acknowledged)
{
    if (!m_sourceLogModel)
        return;

    const QModelIndex proxyIndex = index(proxyRow, 0);
    if (!proxyIndex.isValid())
        return;

    const QModelIndex sourceIndex = mapToSource(proxyIndex);
    if (!sourceIndex.isValid())
        return;

    m_sourceLogModel->setData(sourceIndex, acknowledged, LogModel::AcknowledgedRole);
}

bool LogFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!m_sourceLogModel)
        return false;

    const QModelIndex idx = m_sourceLogModel->index(sourceRow, 0, sourceParent);

    const QString level = m_sourceLogModel->data(idx, LogModel::LevelRole).toString();
    const QString timestamp = m_sourceLogModel->data(idx, LogModel::TimestampRole).toString();
    const QString message = m_sourceLogModel->data(idx, LogModel::MessageRole).toString();
    const bool acknowledged = m_sourceLogModel->data(idx, LogModel::AcknowledgedRole).toBool();

    const bool levelMatch = m_levelFilter.isEmpty() || level == m_levelFilter;

    const QString query = m_searchText.trimmed();
    const bool textMatch = query.isEmpty() || level.contains(query, Qt::CaseInsensitive)
                           || timestamp.contains(query, Qt::CaseInsensitive)
                           || message.contains(query, Qt::CaseInsensitive);

    const bool ackMatch = !m_showOnlyUnacknowledged || !acknowledged;

    return levelMatch && textMatch && ackMatch;
}
