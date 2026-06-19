#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QObject>
#include <QString>
#include <QDir>

class SearchWorker : public QObject {
    Q_OBJECT
public:
    explicit SearchWorker(const QString &startingPath, const QString &searchText, QObject *parent = nullptr);

public slots:
    // This is the function that our dedicated thread will call.
    // It will run the heavy computation without blocking the main screen.
    void performScan();

    // Allows us to cancel a running search instantly if the user types a new word
    void cancelSearch();

signals:
    // This signal streams every matching file we find *as soon as we find it*
    void resultFound(const QString &filePath);

    // This signal tells the UI that we have scanned everything.
    void searchComplete();

private:
    QString m_startingPath;
    QString m_searchText;
    bool m_stopSearch; // Flag we check to see if we should stop.
};

#endif // SEARCHWORKER_H