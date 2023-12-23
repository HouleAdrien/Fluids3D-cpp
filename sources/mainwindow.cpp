#include "../headers/mainwindow.h"
#include "../headers/window.h"
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>

MainWindow::MainWindow()
{
    onAddNew();
    showMaximized();

}

void MainWindow::onAddNew()
{
    if (!centralWidget())
        setCentralWidget(new Window(this));
    else
        QMessageBox::information(0, tr("Cannot add new window"), tr("Already occupied. Undock first."));
}

