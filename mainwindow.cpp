#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "modbuswidget.h"

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Modbus RTU 上位机"));

    auto *layout = new QVBoxLayout(ui->centralwidget);
    m_modbusWidget = new ModbusWidget(ui->centralwidget);
    layout->addWidget(m_modbusWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
