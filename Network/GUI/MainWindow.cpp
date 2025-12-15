#include "MainWindow.hpp"
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* centralWidget = new QWidget(this);

    setCentralWidget(centralWidget);

    auto* layout = new QVBoxLayout(centralWidget);
    _button = new QPushButton("Click Me", this);
    layout->addWidget(_button);

    resize(700, 300);
}
