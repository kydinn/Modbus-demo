#include "modbusworker.h"
#include "modbusserialport.h"

ModbusWorker::ModbusWorker(QObject *parent)
    : QObject(parent)
    , m_serial(new ModbusSerialPort(this))
{
}

ModbusWorker::~ModbusWorker()
{
    if (m_serial->isOpen())
        m_serial->close();
}

void ModbusWorker::openPort(const QString &portName, int baudRate)
{
    bool ok = m_serial->open(portName, baudRate);
    emit portOpened(ok, ok ? QString() : m_serial->lastError());
}

void ModbusWorker::closePort()
{
    m_serial->close();
    emit portClosed();
}

void ModbusWorker::readRegister(int requestId, int slaveAddr, int regAddr)
{
    if (!m_serial->isOpen()) {
        emit readFinished(requestId, false, 0);
        return;
    }
    m_serial->setSlaveAddress(slaveAddr);
    uint16_t value = 0;
    int rc = m_serial->readHoldingRegisters(regAddr, 1, &value);
    emit readFinished(requestId, rc != -1, static_cast<int>(value));
}

void ModbusWorker::writeRegister(int requestId, int slaveAddr, int regAddr, int value)
{
    if (!m_serial->isOpen()) {
        emit writeFinished(requestId, false, QStringLiteral("Port not open"));
        return;
    }
    m_serial->setSlaveAddress(slaveAddr);
    int rc = m_serial->writeSingleRegister(regAddr, static_cast<uint16_t>(value));
    emit writeFinished(requestId, rc != -1,
                       rc == -1 ? m_serial->lastError() : QString());
}
