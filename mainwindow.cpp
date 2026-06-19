#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "navigationmanager.h"
#include "searchworker.h"
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Setup the core file system engine
    QFileSystemModel *model = new QFileSystemModel(this);
    model->setRootPath(QDir::rootPath());

    // 2. Setup the Search Filter Proxy Engine
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(0); // Filters by file/folder Name column

    // 3. Attach models to your newly arranged views
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(QDir::homePath()));

    ui->listView->setModel(proxyModel);
    ui->listView->setRootIndex(proxyModel->mapFromSource(model->index(QDir::homePath())));

    // 4. Initialize our Navigation Manager mini-brain starting at Home
    NavigationManager *navManager = new NavigationManager(QDir::homePath(), this);
    ui->lineEdit->setText(navManager->currentPath());

    // 5. Sync Manager: When directory changes, update layouts smoothly
    connect(navManager, &NavigationManager::pathChanged, this, [=](const QString &newPath) {
        QModelIndex sourceIndex = model->index(newPath);
        ui->listView->setRootIndex(proxyModel->mapFromSource(sourceIndex));
        ui->lineEdit->setText(newPath);
        ui->textEdit->clear(); // Clear out old document reader data when moving folders
    });

    // FEATURE 1: Instant Filter Search Bar (lineEdit_2) - Filters while you type!
    // connect(ui->lineEdit_2, &QLineEdit::returnPressed,
    //         this, [=]() {

    //             QString target = ui->lineEdit_2->text();
    //             QString currentPath = ui->lineEdit->text();

    //             QDir dir(currentPath);

    //             QFileInfoList files =
    //                 dir.entryInfoList(QDir::AllEntries |
    //                                   QDir::NoDotAndDotDot);

    //             for(const QFileInfo &file : files) {
    //                 if(file.fileName().contains(target,
    //                                              Qt::CaseInsensitive)) {

    //                     if(file.isDir()) {
    //                         navManager->navigateTo(
    //                             file.absoluteFilePath()
    //                             );
    //                     }

    //                     break;
    //                 }
    //             }
    //         });
    // 0. Initialize your custom search results model (put this near your other model setup)
    m_searchResultsModel = new QStandardItemModel(this);

    // FEATURE 1: Advanced Deep-Content Multi-Threaded Search Bar (lineEdit_2)
    connect(ui->lineEdit_2, &QLineEdit::returnPressed, this, [=]() {
        QString searchText = ui->lineEdit_2->text();
        QString searchStartPath = ui->lineEdit->text(); // Start scanning from current directory

        // If the search bar is empty, restore normal folder view
        if (searchText.isEmpty()) {
            ui->listView->setModel(proxyModel);
            ui->listView->setRootIndex(proxyModel->mapFromSource(model->index(searchStartPath)));
            return;
        }

        // 1. If an old search is still running, safely cancel it and kill the thread
        if (m_searchThread && m_searchThread->isRunning()) {
            m_searchworker->cancelSearch();
            m_searchThread->quit();
            m_searchThread->wait();
        }

        // 2. Switch our list view to display our custom real-time search results list
        m_searchResultsModel->clear();
        m_searchResultsModel->setHorizontalHeaderLabels(QStringList() << "Search Results (Global Scan)");
        ui->listView->setModel(m_searchResultsModel);
        ui->listView->setRootIndex(QModelIndex()); // Reset root index for custom list

        ui->infoLabel->setText("Scanning directories & contents in background thread...");

        // 3. Spawn our Threading infrastructure
        m_searchThread = new QThread(this);
        m_searchworker = new SearchWorker(searchStartPath, searchText);
        m_searchworker->moveToThread(m_searchThread);

        // 4. Thread Wiring (The Magic Pipeline)
        // When the thread starts, tell the worker to start processing the heavy algorithm loop
        connect(m_searchThread, &QThread::started, m_searchworker, &SearchWorker::performScan);

        // STREAMING: As the background worker finds matching files, safely inject them into the UI live!
        // STREAMING: Clean UI Injection (Shows only the filename, blocks text editing!)
        connect(m_searchworker, &SearchWorker::resultFound, this, [=](const QString &filePath) {
            QFileInfo fileInfo(filePath);
            QString cleanFileName = fileInfo.fileName();

            // 1. Create the item displaying ONLY the clean file/folder name
            QStandardItem *item = new QStandardItem(cleanFileName);

            // 2. Hide the heavy absolute path safely inside background data memory
            item->setData(filePath, Qt::UserRole);

            // 3. CRITICAL RESUME FIX: Disable text editing on double-click!
            // This forces double-clicks to fire our custom navigation/open actions instead.
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            m_searchResultsModel->appendRow(item);
        });

        // CLEANUP: When the search completes, update info label and safely delete worker components
        connect(m_searchworker, &SearchWorker::searchComplete, this, [=]() {
            ui->infoLabel->setText("Search complete! Found " + QString::number(m_searchResultsModel->rowCount()) + " matches.");
            m_searchThread->quit();
        });

        // Memory Guard: Wipe allocations when threads exit execution cleanly
        connect(m_searchThread, &QThread::finished, m_searchworker, &QObject::deleteLater);
        connect(m_searchThread, &QThread::finished, m_searchThread, &QObject::deleteLater);

        // 5. Fire up the engine!
        m_searchThread->start();
    });

    // FEATURE 2: Double-clicking folders in the Tree Sidebar (Left Pane)
    connect(ui->treeView, &QTreeView::doubleClicked, this, [=](const QModelIndex &index) {
        navManager->navigateTo(model->filePath(index));
    });

    // FEATURE 3: Double-clicking items in the Main List View (Right Pane)
    connect(ui->listView, &QListView::doubleClicked, this, [=](const QModelIndex &proxyIndex) {
        QString itemPath;

        // 1. Check if we are interacting with search results or regular file views
        if (ui->listView->model() == m_searchResultsModel) {
            itemPath = proxyIndex.data(Qt::UserRole).toString();
        } else {
            QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
            itemPath = model->filePath(sourceIndex);
        }

        if (itemPath.isEmpty()) return;

        QFileInfo itemInfo(itemPath);

        // 2. Execute target action
        if (itemInfo.isDir()) {
            // Restore normal view so you drop into the folder cleanly
            ui->listView->setModel(proxyModel);
            navManager->navigateTo(itemPath);
        }
        else if (itemInfo.isFile()) {
            // Launch the file externally using native OS applications
            QDesktopServices::openUrl(QUrl::fromLocalFile(itemPath));
        }
    });

    // FEATURE 4: Typing an address manually into the top bar and pressing Enter
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, [=]() {
        navManager->navigateTo(ui->lineEdit->text());
    });

    // FEATURE 5: Browser Button Clicks mapped to your sleek UI arrows
    connect(ui->pushButton, &QPushButton::clicked, this, [=]() {
        navManager->goBack(); // Right arrow button (Forward)
    });
    connect(ui->pushButton_2, &QPushButton::clicked, this, [=]() {
        navManager->goForward();    // Left arrow button (Back)
    });

    // FEATURE 6: File Preview Stream Panel (Single click file to read contents)
    connect(ui->listView, &QListView::clicked, this, [=](const QModelIndex &proxyIndex) {
        if (!proxyIndex.isValid()) return;

        QString itemPath;

        // SAFE GUARD: Check exactly which model the list view is using right now
        if (ui->listView->model() == m_searchResultsModel) {
            // If it's a multi-threaded search result, pull straight from hidden memory data
            itemPath = proxyIndex.data(Qt::UserRole).toString();
        } else {
            // Otherwise, safely translate through the proxy filter layer
            QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
            itemPath = model->filePath(sourceIndex);
        }

        // If for any reason the path data is empty, back out to avoid a crash
        if (itemPath.isEmpty()) return;

        QFileInfo itemInfo(itemPath);

        // Update information labels safely
        ui->infoLabel->setText(
            "Name: " + itemInfo.fileName() + "\n" +
            "Type: " + (itemInfo.isDir() ? QString("Folder") : QString("File")) + "\n" +
            "Size: " + QString::number(itemInfo.size()) + " bytes\n" +
            "Path: " + itemInfo.absoluteFilePath() + "\n" +
            "Modified: " + itemInfo.lastModified().toString()
            );

        // Document Text Streamer
        if (itemInfo.isFile()) {
            QFile file(itemPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                ui->textEdit->setText(in.readAll());
                file.close();
            } else {
                ui->textEdit->setText("--- Cannot open this file type or access is denied ---");
            }
        }
    });

    connect(ui-> pushButton_3, &QPushButton::clicked,
            this, [=]() {

                QString folderName =
                    QInputDialog::getText(
                        this,
                        "New Folder",
                        "Enter folder name:"
                        );

                if(folderName.isEmpty())
                    return;

                QDir dir(ui->lineEdit->text());

                if(dir.mkdir(folderName))
                {
                    ui->infoLabel->setText(
                        "Created folder: " + folderName
                        );
                }
                else
                {
                    ui->infoLabel->setText(
                        "Failed to create folder."
                        );
                }
            });

    // FEATURE 8: Delete File / Folder
    connect(ui->pushButton_4, &QPushButton::clicked,
            this, [=]() {

                QModelIndex proxyIndex = ui->listView->currentIndex();

                if(!proxyIndex.isValid())
                {
                    ui->infoLabel->setText(
                        "Please select a file or folder first."
                        );
                    return;
                }

                QModelIndex sourceIndex =
                    proxyModel->mapToSource(proxyIndex);

                QString itemPath =
                    model->filePath(sourceIndex);

                QFileInfo itemInfo(itemPath);

                QMessageBox::StandardButton reply =
                    QMessageBox::question(
                        this,
                        "Delete",
                        "Delete \"" + itemInfo.fileName() + "\" ?",
                        QMessageBox::Yes | QMessageBox::No
                        );

                if(reply != QMessageBox::Yes)
                    return;

                bool success = false;

                if(itemInfo.isDir())
                {
                    success =
                        QDir(itemPath).removeRecursively();
                }
                else
                {
                    success =
                        QFile::remove(itemPath);
                }

                if(success)
                {
                    ui->infoLabel->setText(
                        "Deleted: " + itemInfo.fileName()
                        );
                }
                else
                {
                    ui->infoLabel->setText(
                        "Delete failed."
                        );
                }
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}