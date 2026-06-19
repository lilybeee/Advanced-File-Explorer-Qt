#ifndef NAVIGATIONMANAGER_H
#define NAVIGATIONMANAGER_H

#include <QObject>
#include <QString>
#include <QStack>

class NavigationManager : public QObject {
    Q_OBJECT
public:
    explicit NavigationManager(const QString &initialPath, QObject *parent = nullptr);

    // Methods we can call from our main window
    void navigateTo(const QString &newPath);
    void goBack();
    void goForward();

    QString currentPath() const;

signals:
    // Signals that tell the main window to update its view
    void pathChanged(const QString &newPath);

private:
    QString m_currentPath;
    QStack<QString> m_backStack;     // Holds history paths when going forward
    QStack<QString> m_forwardStack;  // Holds history paths when going backward
    bool m_isInternalNavigation;     // Stops loops while clicking back/forward buttons
};

#endif // NAVIGATIONMANAGER_H