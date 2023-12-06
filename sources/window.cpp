#include "../headers/window.h"
#include "../headers/glwidget.h"
#include "../headers/mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>


Window::Window(MainWindow *mw)
    : mainWindow(mw)
{
    glWidget = new GLWidget;
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *container = new QHBoxLayout;
    container->addWidget(glWidget);

    // Add the button and connect its click signal to the uploadImage slot
    uploadButton = new QPushButton(tr("Upload Image"));
    connect(uploadButton, &QPushButton::clicked, this, &Window::uploadImage);
    mainLayout->addWidget(uploadButton);

    QWidget *w = new QWidget;
    w->setLayout(container);
    mainLayout->addWidget(w);



    setLayout(mainLayout);
    setWindowTitle(tr("Qt OpenGL"));
}

Window::~Window() {
    delete glWidget;
}

void Window::uploadImage() {
    // Open a file dialog to select an image file
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image File"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    QImage _image(filePath);
    // Do something with the selected image file path, e.g., pass it to GLWidget for rendering
    if(!_image.isNull() && _image.isGrayscale()){
        glWidget->grid->setHeightMap(_image);
    }else{
        QMessageBox::critical(this, tr("Error"), tr("Please select a black and white image."));

    }


}
