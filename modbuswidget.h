#ifndef MODBUSWIDGET_H
#define MODBUSWIDGET_H

#include <QWidget>

class QComboBox;
class QPushButton;
class QLabel;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QTimer;
class QThread;
class ModbusWorker;
class ModbusCommand;

class ModbusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModbusWidget(QWidget *parent = nullptr);
    ~ModbusWidget();

    ModbusCommand *command() const;
    bool isPortOpen() const { return m_portOpen; }
    void writeRegister(int address, int value);

private slots:
    void scanPorts();
    void togglePort();
    void onEnableToggled(bool checked);
    void onTimerTimeout();
    void onWriteRegister1();
    void onWriteRegister2();

    void onPortOpened(bool success, const QString &error);
    void onPortClosed();
    void onReadFinished(int requestId, bool success, int value);
    void onWriteFinished(int requestId, bool success, const QString &error);

private:
    void setupUi();
    void updateToggleButton(bool connected);
    void setPortOpen(bool open);

    // --- 串口设置 ---
    QComboBox *m_portCombo = nullptr;
    QPushButton *m_scanBtn = nullptr;
    QComboBox *m_baudRateCombo = nullptr;
    QPushButton *m_toggleBtn = nullptr;
    QLabel *m_statusLabel = nullptr;

    // --- 通信参数 ---
    QSpinBox *m_slaveAddrSpin = nullptr;
    QSpinBox *m_intervalSpin = nullptr;
    QCheckBox *m_enableCheck = nullptr;

    // --- 写入示例 ---
    QSpinBox *m_writeAddrSpin1 = nullptr;
    QSpinBox *m_writeValueSpin1 = nullptr;
    QPushButton *m_writeBtn1 = nullptr;

    QSpinBox *m_writeAddrSpin2 = nullptr;
    QSpinBox *m_writeValueSpin2 = nullptr;
    QPushButton *m_writeBtn2 = nullptr;

    // --- 读取示例 ---
    QSpinBox *m_readAddrSpin1 = nullptr;
    QLabel *m_readResult1 = nullptr;

    QSpinBox *m_readAddrSpin2 = nullptr;
    QLabel *m_readResult2 = nullptr;

    // --- 核心对象 ---
    QThread *m_workerThread = nullptr;
    ModbusWorker *m_worker = nullptr;
    ModbusCommand *m_command = nullptr;
    QTimer *m_pollTimer = nullptr;
    bool m_portOpen = false;
    bool m_readPending = false;
};

#endif // MODBUSWIDGET_H
