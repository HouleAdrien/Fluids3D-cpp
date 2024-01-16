#include "../headers/window.h"
#include "../headers/glwidget.h"
#include "../headers/mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSpacerItem>
#include <QComboBox>
#include <QLabel>

Window::Window(MainWindow *mw) : mainWindow(mw) {
    glWidget = new GLWidget;
    setAttribute(Qt::WA_DeleteOnClose);

    QHBoxLayout *container = new QHBoxLayout; // Layout principal

    QVBoxLayout *dropdownLayout = new QVBoxLayout; // Layout pour les dropdowns et labels

    // Créer le premier menu déroulant et label
    QLabel *label1 = new QLabel(tr("Scène :"));
    QComboBox *dropdown1 = new QComboBox;
    dropdown1->addItems(QStringList{"rivière", "îles", "canaux", "pas de terrain"});
    dropdownLayout->addWidget(label1);
    dropdownLayout->addWidget(dropdown1);
    connect(dropdown1, SIGNAL(currentIndexChanged(int)), this, SLOT(onFirstDropdownChanged(int)));
/*
    // Créer le deuxième menu déroulant et label
    QLabel *label2 = new QLabel(tr("Bordure d'emission du courant continu :"));
    QComboBox *dropdown2 = new QComboBox;
    dropdown2->addItems(QStringList{"Nord", "Sud", "Est", "Ouest"});
    dropdownLayout->addWidget(label2);
    dropdownLayout->addWidget(dropdown2);
    connect(dropdown2, SIGNAL(currentIndexChanged(int)), this, SLOT(onSecondDropdownChanged(int)));*/


    QSpacerItem* verticalSpacer = new QSpacerItem(20, 300, QSizePolicy::Minimum, QSizePolicy::Expanding);
    dropdownLayout->addItem(verticalSpacer); // Ajoutez l'espaceur en bas

    // Ajouter glWidget et le layout vertical au layout horizontal
    container->addWidget(glWidget);
    container->addLayout(dropdownLayout);

    // Ajuster les proportions entre glWidget et le layout de dropdown
    container->setStretchFactor(glWidget, 3); // 75% pour glWidget
    container->setStretchFactor(dropdownLayout, 1); // 25% pour les dropdowns et labels

    QVBoxLayout *mainLayout = new QVBoxLayout; // Layout général
    mainLayout->addLayout(container);

    setLayout(mainLayout);
    setWindowTitle(tr("Qt OpenGL"));
    showMaximized();
}


// Slot for first dropdown value change
void Window::onFirstDropdownChanged(int index) {
    glWidget->ChangeTerrain(index);
}

// Slot for second dropdown value change
void Window::onSecondDropdownChanged(int index) {
    // Handle the change here
}

Window::~Window() {
    delete glWidget;
}

