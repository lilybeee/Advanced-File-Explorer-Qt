#include "searchworker.h"
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QDebug>

SearchWorker::SearchWorker(const QString &startingPath, const QString &searchText, QObject *parent)
    : QObject(parent), m_startingPath(startingPath), m_searchText(searchText), m_stopSearch(false) {}

void SearchWorker::cancelSearch() {
    m_stopSearch = true; // Set flag so the loop exits immediately
}

void SearchWorker::performScan() {
    if (m_searchText.isEmpty()) {
        emit searchComplete();
        return;
    }

    // High-performance directory navigator to crawl subfolders safely
    QDirIterator it(m_startingPath, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    while (it.hasNext() && !m_stopSearch) {
        it.next();
        QFileInfo itemInfo = it.fileInfo();

        // LASER FOCUS: Only match if the file or folder name itself contains your search keyword
        if (itemInfo.fileName().contains(m_searchText, Qt::CaseInsensitive)) {
            emit resultFound(itemInfo.absoluteFilePath());
        }
    }

    if (!m_stopSearch) {
        emit searchComplete();
    }
}