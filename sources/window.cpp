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

    imageLabel = new QLabel;
    imageLabel->setPixmap(QPixmap(":/images/test4.png"));
    imageLabel->setMinimumSize(200, 200);
    container->addWidget(imageLabel);

    imageLabel2 = new QLabel;
    imageLabel2->setPixmap(QPixmap(":/images/test4.png"));
    imageLabel2->setMinimumSize(200, 200);
    container->addWidget(imageLabel2);

    container->addWidget(glWidget);

    // Add the button and connect its click signal to the uploadImage slot
    uploadButton = new QPushButton(tr("Upload Image"));
    connect(uploadButton, &QPushButton::clicked, this, &Window::uploadImage);
    mainLayout->addWidget(uploadButton);

    QWidget *w = new QWidget;
    w->setLayout(container);
    mainLayout->addWidget(w);

    connect(glWidget, &GLWidget::reflectionTextureUpdated, this, &Window::updateImageDisplay);

    connect(glWidget, &GLWidget::refractionTextureUpdated, this, &Window::updateImageDisplay2);
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

void Window::updateImageDisplay(const QImage &image) {
    imageLabel->setPixmap(QPixmap::fromImage(image));
}

void Window::updateImageDisplay2(const QImage &image) {
    imageLabel2->setPixmap(QPixmap::fromImage(image));
}
