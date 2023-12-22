#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QLabel>
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
    void updateImageDisplay(const QImage &image);
    void updateImageDisplay2(const QImage &image);

private:
    QLabel *imageLabel;
    QLabel *imageLabel2;

    GLWidget *glWidget;
    MainWindow *mainWindow;
    QPushButton *uploadButton;
};

#endif

