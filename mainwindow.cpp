#include "mainwindow.h"
#include "glwidget.h"
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    m_glWidget = new GLWidget(this);
    layout->addWidget(m_glWidget);
    setCentralWidget(central);

    resize(800, 600);
    setWindowTitle("OBJ Viewer - Qt OpenGL");
}

MainWindow::~MainWindow() {}
