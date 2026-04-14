#ifndef MODBUSWORKER_H
#define MODBUSWORKER_H

#include <QObject>
#include <QString>
#include <cstdint>

class ModbusSerialPort;

class ModbusWorker : public QObject
{
    Q_OBJECT

public:
    explicit ModbusWorker(QObject *parent = nullptr);
    ~ModbusWorker();

public slots:
    void openPort(const QString &portName, int baudRate);
    void closePort();
    void readRegister(int requestId, int slaveAddr, int regAddr);
    void writeRegister(int requestId, int slaveAddr, int regAddr, int value);

signals:
    void portOpened(bool success, const QString &error);
    void portClosed();
    void readFinished(int requestId, bool success, int value);
    void writeFinished(int requestId, bool success, const QString &error);

private:
    ModbusSerialPort *m_serial = nullptr;
};

#endif // MODBUSWORKER_H
