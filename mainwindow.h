#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QThread>
#include <QStandardItemModel>
#include "searchworker.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    QThread *m_searchThread = nullptr;
    SearchWorker *m_searchworker = nullptr;
    QStandardItemModel *m_searchResultsModel = nullptr;
};
#endif // MAINWINDOW_H
