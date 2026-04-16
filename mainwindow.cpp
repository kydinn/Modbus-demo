#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "modbuswidget.h"
#include "alarmdisplaywidget.h"
#include "alarmconfigwidget.h"

#include <QVBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Modbus RTU 上位机"));

    auto *layout = new QVBoxLayout(ui->centralwidget);
    m_modbusWidget = new ModbusWidget(ui->centralwidget);
    layout->addWidget(m_modbusWidget);

    auto *alarmWidget = new AlarmDisplayWidget(ui->centralwidget);
    layout->addWidget(alarmWidget);

    auto *alarmConfig = new AlarmConfigWidget(ui->centralwidget);
    layout->addWidget(alarmConfig);

    connect(alarmConfig, &AlarmConfigWidget::writeRequested,
            this, [this](int value1, int value2) {
        if (!m_modbusWidget->isPortOpen()) {
            QMessageBox::warning(this, QStringLiteral("警告"),
                                  QStringLiteral("请先打开串口"));
            return;
        }
        m_modbusWidget->writeRegister(0, value1);
        m_modbusWidget->writeRegister(1, value2);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
