#include "modbuscommand.h"
#include "modbusserialport.h"

ModbusCommand::ModbusCommand(QObject *parent)
    : QObject(parent)
{
}

// --- 标准协议字段 getter / setter ---

void ModbusCommand::setSlaveAddress(uint8_t address) { m_slaveAddress = address; }
uint8_t ModbusCommand::slaveAddress() const { return m_slaveAddress; }

void ModbusCommand::setFunctionCode(FunctionCode code) { m_functionCode = code; }
ModbusCommand::FunctionCode ModbusCommand::functionCode() const { return m_functionCode; }

void ModbusCommand::setStartAddress(uint16_t address) { m_startAddress = address; }
uint16_t ModbusCommand::startAddress() const { return m_startAddress; }

void ModbusCommand::setQuantity(uint16_t quantity) { m_quantity = quantity; }
uint16_t ModbusCommand::quantity() const { return m_quantity; }

void ModbusCommand::setWriteData(const QVector<uint16_t> &data) { m_writeData = data; }
QVector<uint16_t> ModbusCommand::writeData() const { return m_writeData; }

void ModbusCommand::setWriteBits(const QVector<uint8_t> &bits) { m_writeBits = bits; }
QVector<uint8_t> ModbusCommand::writeBits() const { return m_writeBits; }

// --- 自定义扩展字段 ---

void ModbusCommand::setCustomField(const QString &name, const QVariant &value)
{
    m_customFields[name] = value;
}

QVariant ModbusCommand::customField(const QString &name,
                                     const QVariant &defaultValue) const
{
    return m_customFields.value(name, defaultValue);
}

QStringList ModbusCommand::customFieldNames() const
{
    return m_customFields.keys();
}

// --- 执行指令 ---

bool ModbusCommand::execute(ModbusSerialPort *port)
{
    if (!port || !port->isOpen()) {
        m_lastError = QStringLiteral("Serial port is not open");
        emit executed(false);
        return false;
    }

    m_responseRegisters.clear();
    m_responseCoils.clear();

    port->setSlaveAddress(m_slaveAddress);

    int rc = -1;
    bool success = false;

    switch (m_functionCode) {
    case ReadCoils: {
        m_responseCoils.resize(m_quantity);
        rc = port->readCoils(m_startAddress, m_quantity, m_responseCoils.data());
        success = (rc != -1);
        break;
    }
    case ReadDiscreteInputs: {
        m_responseCoils.resize(m_quantity);
        rc = port->readDiscreteInputs(m_startAddress, m_quantity,
                                       m_responseCoils.data());
        success = (rc != -1);
        break;
    }
    case ReadHoldingRegisters: {
        m_responseRegisters.resize(m_quantity);
        rc = port->readHoldingRegisters(m_startAddress, m_quantity,
                                         m_responseRegisters.data());
        success = (rc != -1);
        break;
    }
    case ReadInputRegisters: {
        m_responseRegisters.resize(m_quantity);
        rc = port->readInputRegisters(m_startAddress, m_quantity,
                                       m_responseRegisters.data());
        success = (rc != -1);
        break;
    }
    case WriteSingleCoil: {
        int status = (!m_writeBits.isEmpty() && m_writeBits.first()) ? 1 : 0;
        rc = port->writeSingleCoil(m_startAddress, status);
        success = (rc != -1);
        break;
    }
    case WriteSingleRegister: {
        uint16_t value = (!m_writeData.isEmpty()) ? m_writeData.first() : 0;
        rc = port->writeSingleRegister(m_startAddress, value);
        success = (rc != -1);
        break;
    }
    case WriteMultipleCoils: {
        if (m_writeBits.isEmpty()) {
            m_lastError = QStringLiteral("No coil data to write");
            emit executed(false);
            return false;
        }
        rc = port->writeCoils(m_startAddress, m_writeBits.size(),
                               m_writeBits.constData());
        success = (rc != -1);
        break;
    }
    case WriteMultipleRegisters: {
        if (m_writeData.isEmpty()) {
            m_lastError = QStringLiteral("No register data to write");
            emit executed(false);
            return false;
        }
        rc = port->writeRegisters(m_startAddress, m_writeData.size(),
                                    m_writeData.constData());
        success = (rc != -1);
        break;
    }
    }

    if (!success) {
        m_lastError = port->lastError();
    } else {
        m_lastError.clear();
    }

    emit executed(success);
    return success;
}

// --- 响应数据 ---

QVector<uint16_t> ModbusCommand::responseRegisters() const { return m_responseRegisters; }
QVector<uint8_t> ModbusCommand::responseCoils() const { return m_responseCoils; }
QString ModbusCommand::lastError() const { return m_lastError; }
