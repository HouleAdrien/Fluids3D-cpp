#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSlider;
class QPushButton;
QT_END_NAMESPACE

class GLWidget;
class MainWindow;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(MainWindow *mw);
    ~Window();

private slots:
    void uploadImage();

private:
    GLWidget *glWidget;
    MainWindow *mainWindow;
    QPushButton *uploadButton;
};

#endif

