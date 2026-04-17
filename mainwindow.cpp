#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "modbuswidget.h"
#include "alarmdisplaywidget.h"
#include "alarmconfigwidget.h"

#include "dragreorderlayout.h"

#include <QVBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Modbus RTU 上位机"));

    auto *outerLayout = new QVBoxLayout(ui->centralwidget);
    auto *dragContainer = new DragReorderLayout(ui->centralwidget);
    outerLayout->addWidget(dragContainer);

    m_modbusWidget = new ModbusWidget(dragContainer);
    dragContainer->addWidget(m_modbusWidget);

    auto *alarmWidget = new AlarmDisplayWidget(dragContainer);
    dragContainer->addWidget(alarmWidget);

    auto *alarmConfig = new AlarmConfigWidget(dragContainer);
    dragContainer->addWidget(alarmConfig);

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
