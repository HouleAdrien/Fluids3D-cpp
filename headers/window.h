#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
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
    void onFirstDropdownChanged(int index);
    void onSecondDropdownChanged(int index);

private:
    GLWidget *glWidget;
    MainWindow *mainWindow;

    // Declare the dropdowns
    QComboBox *dropdown1;
    QComboBox *dropdown2;
};

#endif
