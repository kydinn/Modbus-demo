#include "modbuswidget.h"
#include "modbusworker.h"
#include "modbuscommand.h"

#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QTimer>
#include <QThread>
#include <QSerialPortInfo>
#include <QMessageBox>

ModbusWidget::ModbusWidget(QWidget *parent)
    : QWidget(parent)
    , m_command(new ModbusCommand(this))
    , m_pollTimer(new QTimer(this))
    , m_workerThread(new QThread(this))
    , m_worker(new ModbusWorker)          // no parent — will be moved to thread
{
    setupUi();
    scanPorts();

    m_worker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    // Worker → Widget (auto queued, cross-thread)
    connect(m_worker, &ModbusWorker::portOpened,
            this, &ModbusWidget::onPortOpened);
    connect(m_worker, &ModbusWorker::portClosed,
            this, &ModbusWidget::onPortClosed);
    connect(m_worker, &ModbusWorker::readFinished,
            this, &ModbusWidget::onReadFinished);
    connect(m_worker, &ModbusWorker::writeFinished,
            this, &ModbusWidget::onWriteFinished);

    // UI
    connect(m_scanBtn, &QPushButton::clicked,
            this, &ModbusWidget::scanPorts);
    connect(m_toggleBtn, &QPushButton::clicked,
            this, &ModbusWidget::togglePort);
    connect(m_enableCheck, &QCheckBox::toggled,
            this, &ModbusWidget::onEnableToggled);
    connect(m_pollTimer, &QTimer::timeout,
            this, &ModbusWidget::onTimerTimeout);
    connect(m_writeBtn1, &QPushButton::clicked,
            this, &ModbusWidget::onWriteRegister1);
    connect(m_writeBtn2, &QPushButton::clicked,
            this, &ModbusWidget::onWriteRegister2);

    connect(m_intervalSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int ms) {
        if (m_pollTimer->isActive())
            m_pollTimer->setInterval(ms);
    });

    m_workerThread->start();
}

ModbusWidget::~ModbusWidget()
{
    m_pollTimer->stop();
    QMetaObject::invokeMethod(m_worker, "closePort", Qt::BlockingQueuedConnection);
    m_workerThread->quit();
    m_workerThread->wait();
}

ModbusCommand *ModbusWidget::command() const
{
    return m_command;
}

void ModbusWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // ===== 串口连接设置 =====
    auto *connGroup = new QGroupBox(QStringLiteral("Modbus RTU 串口设置"), this);
    auto *connForm = new QFormLayout;

    auto *portLayout = new QHBoxLayout;
    m_portCombo = new QComboBox;
    m_portCombo->setMinimumWidth(160);
    m_scanBtn = new QPushButton(QStringLiteral("扫描串口"));
    portLayout->addWidget(m_portCombo, 1);
    portLayout->addWidget(m_scanBtn);
    connForm->addRow(QStringLiteral("串口:"), portLayout);

    m_baudRateCombo = new QComboBox;
    const QList<int> baudRates = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    for (int br : baudRates) {
        m_baudRateCombo->addItem(QString::number(br), br);
    }
    m_baudRateCombo->setCurrentText(QStringLiteral("9600"));
    connForm->addRow(QStringLiteral("波特率:"), m_baudRateCombo);

    connGroup->setLayout(connForm);
    mainLayout->addWidget(connGroup);

    m_toggleBtn = new QPushButton(QStringLiteral("打开串口"));
    m_toggleBtn->setCheckable(true);
    m_toggleBtn->setMinimumHeight(36);
    mainLayout->addWidget(m_toggleBtn);

    // ===== 通信参数 =====
    auto *paramGroup = new QGroupBox(QStringLiteral("通信参数"), this);
    auto *paramForm = new QFormLayout;

    m_slaveAddrSpin = new QSpinBox;
    m_slaveAddrSpin->setRange(1, 247);
    m_slaveAddrSpin->setValue(1);
    paramForm->addRow(QStringLiteral("从站地址:"), m_slaveAddrSpin);

    m_intervalSpin = new QSpinBox;
    m_intervalSpin->setRange(50, 10000);
    m_intervalSpin->setValue(1000);
    m_intervalSpin->setSuffix(QStringLiteral(" ms"));
    m_intervalSpin->setSingleStep(100);
    paramForm->addRow(QStringLiteral("通信间隔:"), m_intervalSpin);

    m_enableCheck = new QCheckBox(QStringLiteral("启用 Modbus 周期通信"));
    paramForm->addRow(m_enableCheck);

    paramGroup->setLayout(paramForm);
    mainLayout->addWidget(paramGroup);

    // ===== 写入示例 =====
    auto *writeGroup = new QGroupBox(QStringLiteral("写入寄存器（示例）"), this);
    auto *writeLayout = new QVBoxLayout;

    // 写入 1
    auto *w1Layout = new QHBoxLayout;
    w1Layout->addWidget(new QLabel(QStringLiteral("地址:")));
    m_writeAddrSpin1 = new QSpinBox;
    m_writeAddrSpin1->setRange(0, 65535);
    m_writeAddrSpin1->setPrefix(QStringLiteral("0x"));
    m_writeAddrSpin1->setDisplayIntegerBase(16);
    m_writeAddrSpin1->setValue(0);
    w1Layout->addWidget(m_writeAddrSpin1);
    w1Layout->addWidget(new QLabel(QStringLiteral("值:")));
    m_writeValueSpin1 = new QSpinBox;
    m_writeValueSpin1->setRange(0, 65535);
    m_writeValueSpin1->setValue(0);
    w1Layout->addWidget(m_writeValueSpin1);
    m_writeBtn1 = new QPushButton(QStringLiteral("写入"));
    w1Layout->addWidget(m_writeBtn1);
    writeLayout->addLayout(w1Layout);

    // 写入 2
    auto *w2Layout = new QHBoxLayout;
    w2Layout->addWidget(new QLabel(QStringLiteral("地址:")));
    m_writeAddrSpin2 = new QSpinBox;
    m_writeAddrSpin2->setRange(0, 65535);
    m_writeAddrSpin2->setPrefix(QStringLiteral("0x"));
    m_writeAddrSpin2->setDisplayIntegerBase(16);
    m_writeAddrSpin2->setValue(1);
    w2Layout->addWidget(m_writeAddrSpin2);
    w2Layout->addWidget(new QLabel(QStringLiteral("值:")));
    m_writeValueSpin2 = new QSpinBox;
    m_writeValueSpin2->setRange(0, 65535);
    m_writeValueSpin2->setValue(0);
    w2Layout->addWidget(m_writeValueSpin2);
    m_writeBtn2 = new QPushButton(QStringLiteral("写入"));
    w2Layout->addWidget(m_writeBtn2);
    writeLayout->addLayout(w2Layout);

    writeGroup->setLayout(writeLayout);
    mainLayout->addWidget(writeGroup);

    // ===== 读取示例 =====
    auto *readGroup = new QGroupBox(QStringLiteral("读取寄存器（示例）"), this);
    auto *readLayout = new QVBoxLayout;

    // 读取 1
    auto *r1Layout = new QHBoxLayout;
    r1Layout->addWidget(new QLabel(QStringLiteral("地址:")));
    m_readAddrSpin1 = new QSpinBox;
    m_readAddrSpin1->setRange(0, 65535);
    m_readAddrSpin1->setPrefix(QStringLiteral("0x"));
    m_readAddrSpin1->setDisplayIntegerBase(16);
    m_readAddrSpin1->setValue(0);
    r1Layout->addWidget(m_readAddrSpin1);
    r1Layout->addWidget(new QLabel(QStringLiteral("结果:")));
    m_readResult1 = new QLabel(QStringLiteral("--"));
    m_readResult1->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_readResult1->setMinimumWidth(80);
    r1Layout->addWidget(m_readResult1, 1);
    readLayout->addLayout(r1Layout);

    // 读取 2
    auto *r2Layout = new QHBoxLayout;
    r2Layout->addWidget(new QLabel(QStringLiteral("地址:")));
    m_readAddrSpin2 = new QSpinBox;
    m_readAddrSpin2->setRange(0, 65535);
    m_readAddrSpin2->setPrefix(QStringLiteral("0x"));
    m_readAddrSpin2->setDisplayIntegerBase(16);
    m_readAddrSpin2->setValue(1);
    r2Layout->addWidget(m_readAddrSpin2);
    r2Layout->addWidget(new QLabel(QStringLiteral("结果:")));
    m_readResult2 = new QLabel(QStringLiteral("--"));
    m_readResult2->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_readResult2->setMinimumWidth(80);
    r2Layout->addWidget(m_readResult2, 1);
    readLayout->addLayout(r2Layout);

    readGroup->setLayout(readLayout);
    mainLayout->addWidget(readGroup);

    // ===== 状态栏 =====
    m_statusLabel = new QLabel(QStringLiteral("状态: 未连接"));
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();
}

void ModbusWidget::scanPorts()
{
    m_portCombo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        QString label = QStringLiteral("%1  (%2)")
                            .arg(info.portName(), info.description());
        m_portCombo->addItem(label, info.portName());
    }

    if (m_portCombo->count() == 0) {
        m_statusLabel->setText(QStringLiteral("状态: 未发现可用串口"));
    } else {
        m_statusLabel->setText(
            QStringLiteral("状态: 发现 %1 个串口").arg(m_portCombo->count()));
    }
}

void ModbusWidget::togglePort()
{
    if (m_portOpen) {
        QMetaObject::invokeMethod(m_worker, "closePort", Qt::QueuedConnection);
    } else {
        if (m_portCombo->count() == 0) {
            QMessageBox::warning(this,
                                  QStringLiteral("警告"),
                                  QStringLiteral("没有可用的串口，请先扫描"));
            m_toggleBtn->setChecked(false);
            return;
        }

        QString portName = m_portCombo->currentData().toString();
        int baudRate = m_baudRateCombo->currentData().toInt();
        QMetaObject::invokeMethod(m_worker, "openPort", Qt::QueuedConnection,
                                  Q_ARG(QString, portName), Q_ARG(int, baudRate));
    }
}

void ModbusWidget::onPortOpened(bool success, const QString &error)
{
    if (success) {
        setPortOpen(true);
    } else {
        m_toggleBtn->setChecked(false);
        m_statusLabel->setText(QStringLiteral("错误: %1").arg(error));
    }
}

void ModbusWidget::onPortClosed()
{
    setPortOpen(false);
}

void ModbusWidget::setPortOpen(bool open)
{
    m_portOpen = open;
    updateToggleButton(open);

    m_portCombo->setEnabled(!open);
    m_baudRateCombo->setEnabled(!open);
    m_scanBtn->setEnabled(!open);

    if (!open) {
        m_enableCheck->setChecked(false);
        m_readPending = false;
        m_readResult1->setText(QStringLiteral("--"));
        m_readResult2->setText(QStringLiteral("--"));
        m_statusLabel->setText(QStringLiteral("状态: 未连接"));
    } else {
        m_statusLabel->setText(
            QStringLiteral("状态: 已连接 - %1 @ %2")
                .arg(m_portCombo->currentData().toString(),
                     m_baudRateCombo->currentText()));
    }
}

void ModbusWidget::onEnableToggled(bool checked)
{
    if (checked) {
        if (!m_portOpen) {
            QMessageBox::warning(this,
                                  QStringLiteral("警告"),
                                  QStringLiteral("请先打开串口"));
            m_enableCheck->setChecked(false);
            return;
        }
        m_pollTimer->start(m_intervalSpin->value());
        m_statusLabel->setText(QStringLiteral("状态: 周期通信已启动 (间隔 %1 ms)")
                                   .arg(m_intervalSpin->value()));
    } else {
        m_pollTimer->stop();
        m_readPending = false;
        if (m_portOpen) {
            m_statusLabel->setText(
                QStringLiteral("状态: 已连接 - %1 @ %2")
                    .arg(m_portCombo->currentData().toString(),
                         m_baudRateCombo->currentText()));
        }
    }
}

void ModbusWidget::onTimerTimeout()
{
    if (!m_portOpen || m_readPending)
        return;

    m_readPending = true;
    int slaveAddr = m_slaveAddrSpin->value();

    QMetaObject::invokeMethod(m_worker, "readRegister", Qt::QueuedConnection,
                              Q_ARG(int, 0), Q_ARG(int, slaveAddr),
                              Q_ARG(int, m_readAddrSpin1->value()));
    QMetaObject::invokeMethod(m_worker, "readRegister", Qt::QueuedConnection,
                              Q_ARG(int, 1), Q_ARG(int, slaveAddr),
                              Q_ARG(int, m_readAddrSpin2->value()));
}

void ModbusWidget::onReadFinished(int requestId, bool success, int value)
{
    if (requestId == 0) {
        m_readResult1->setText(success ? QString::number(value)
                                       : QStringLiteral("ERR"));
    } else if (requestId == 1) {
        m_readResult2->setText(success ? QString::number(value)
                                       : QStringLiteral("ERR"));
        m_readPending = false;
    }
}

void ModbusWidget::onWriteRegister1()
{
    if (!m_portOpen) {
        QMessageBox::warning(this, QStringLiteral("警告"),
                              QStringLiteral("请先打开串口"));
        return;
    }
    m_writeBtn1->setEnabled(false);
    QMetaObject::invokeMethod(m_worker, "writeRegister", Qt::QueuedConnection,
                              Q_ARG(int, 0),
                              Q_ARG(int, m_slaveAddrSpin->value()),
                              Q_ARG(int, m_writeAddrSpin1->value()),
                              Q_ARG(int, m_writeValueSpin1->value()));
}

void ModbusWidget::onWriteRegister2()
{
    if (!m_portOpen) {
        QMessageBox::warning(this, QStringLiteral("警告"),
                              QStringLiteral("请先打开串口"));
        return;
    }
    m_writeBtn2->setEnabled(false);
    QMetaObject::invokeMethod(m_worker, "writeRegister", Qt::QueuedConnection,
                              Q_ARG(int, 1),
                              Q_ARG(int, m_slaveAddrSpin->value()),
                              Q_ARG(int, m_writeAddrSpin2->value()),
                              Q_ARG(int, m_writeValueSpin2->value()));
}

void ModbusWidget::onWriteFinished(int requestId, bool success, const QString &error)
{
    QSpinBox *addrSpin = nullptr;
    QSpinBox *valSpin = nullptr;
    QPushButton *btn = nullptr;

    if (requestId == 0) {
        addrSpin = m_writeAddrSpin1;
        valSpin  = m_writeValueSpin1;
        btn      = m_writeBtn1;
    } else {
        addrSpin = m_writeAddrSpin2;
        valSpin  = m_writeValueSpin2;
        btn      = m_writeBtn2;
    }

    btn->setEnabled(true);

    if (success) {
        m_statusLabel->setText(QStringLiteral("写入成功: 地址 0x%1 = %2")
                                   .arg(addrSpin->value(), 4, 16, QLatin1Char('0'))
                                   .arg(valSpin->value()));
    } else {
        m_statusLabel->setText(QStringLiteral("写入失败: %1").arg(error));
    }
}

void ModbusWidget::updateToggleButton(bool connected)
{
    m_toggleBtn->setChecked(connected);
    m_toggleBtn->setText(connected ? QStringLiteral("关闭串口")
                                    : QStringLiteral("打开串口"));
    m_toggleBtn->setStyleSheet(
        connected
            ? QStringLiteral("QPushButton { background-color: #e74c3c; color: white; }")
            : QString());
}
