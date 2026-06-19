#include "navigationmanager.h"

NavigationManager::NavigationManager(const QString &initialPath, QObject *parent)
    : QObject(parent), m_currentPath(initialPath), m_isInternalNavigation(false) {}

// Standard web-browser logic for history navigation
void NavigationManager::navigateTo(const QString &newPath) {
    if (newPath == m_currentPath || newPath.isEmpty()) return;

    // If the user clicked a new folder manually, save old path to back stack
    // and clear out the forward history
    if (!m_isInternalNavigation) {
        m_backStack.push(m_currentPath);
        m_forwardStack.clear();
    }

    m_currentPath = newPath;
    emit pathChanged(m_currentPath); // Tell mainwindow to update the screen
}

void NavigationManager::goBack() {
    if (m_backStack.isEmpty()) return;

    m_forwardStack.push(m_currentPath);

    m_isInternalNavigation = true;
    navigateTo(m_backStack.pop()); // Jump back one step
    m_isInternalNavigation = false;
}

void NavigationManager::goForward() {
    if (m_forwardStack.isEmpty()) return;

    m_backStack.push(m_currentPath);

    m_isInternalNavigation = true;
    navigateTo(m_forwardStack.pop()); // Jump forward one step
    m_isInternalNavigation = false;
}

QString NavigationManager::currentPath() const {
    return m_currentPath;
}