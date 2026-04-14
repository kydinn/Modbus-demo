#include "modbusserialport.h"

ModbusSerialPort::ModbusSerialPort(QObject *parent)
    : QObject(parent)
{
}

ModbusSerialPort::~ModbusSerialPort()
{
    close();
}

bool ModbusSerialPort::open(const QString &portName, int baudRate,
                             char parity, int dataBits, int stopBits)
{
    close();

#ifdef _WIN32
    QString device = portName;
    if (!device.startsWith("\\\\.\\")) {
        device = "\\\\.\\" + portName;
    }
#else
    const QString &device = portName;
#endif

    m_ctx = modbus_new_rtu(device.toLocal8Bit().constData(),
                           baudRate, parity, dataBits, stopBits);
    if (!m_ctx) {
        m_lastError = QStringLiteral("Failed to create modbus RTU context: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
        return false;
    }

    if (modbus_connect(m_ctx) == -1) {
        m_lastError = QStringLiteral("Failed to connect: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
        modbus_free(m_ctx);
        m_ctx = nullptr;
        return false;
    }

    m_isOpen = true;
    emit connectionStateChanged(true);
    return true;
}

void ModbusSerialPort::close()
{
    if (m_ctx) {
        if (m_isOpen) {
            modbus_close(m_ctx);
        }
        modbus_free(m_ctx);
        m_ctx = nullptr;
    }
    if (m_isOpen) {
        m_isOpen = false;
        emit connectionStateChanged(false);
    }
}

bool ModbusSerialPort::isOpen() const
{
    return m_isOpen;
}

void ModbusSerialPort::setSlaveAddress(int address)
{
    if (m_ctx) {
        modbus_set_slave(m_ctx, address);
    }
}

void ModbusSerialPort::setResponseTimeout(uint32_t milliseconds)
{
    if (m_ctx) {
        modbus_set_response_timeout(m_ctx, milliseconds / 1000,
                                     (milliseconds % 1000) * 1000);
    }
}

int ModbusSerialPort::readHoldingRegisters(int addr, int nb, uint16_t *dest)
{
    if (!m_ctx || !m_isOpen) {
        m_lastError = QStringLiteral("Port not open");
        return -1;
    }
    int rc = modbus_read_registers(m_ctx, addr, nb, dest);
    if (rc == -1) {
        m_lastError = QStringLiteral("Read holding registers failed: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
    }
    return rc;
}

int ModbusSerialPort::readInputRegisters(int addr, int nb, uint16_t *dest)
{
    if (!m_ctx || !m_isOpen) {
        m_lastError = QStringLiteral("Port not open");
        return -1;
    }
    int rc = modbus_read_input_registers(m_ctx, addr, nb, dest);
    if (rc == -1) {
        m_lastError = QStringLiteral("Read input registers failed: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
    }
    return rc;
}

int ModbusSerialPort::readCoils(int addr, int nb, uint8_t *dest)
{
    if (!m_ctx || !m_isOpen) {
        m_lastError = QStringLiteral("Port not open");
        return -1;
    }
    int rc = modbus_read_bits(m_ctx, addr, nb, dest);
    if (rc == -1) {
        m_lastError = QStringLiteral("Read coils failed: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
    }
    return rc;
}

int ModbusSerialPort::readDiscreteInputs(int addr, int nb, uint8_t *dest)
{
    if (!m_ctx || !m_isOpen) {
        m_lastError = QStringLiteral("Port not open");
        return -1;
    }
    int rc = modbus_read_input_bits(m_ctx, addr, nb, dest);
    if (rc == -1) {
        m_lastError = QStringLiteral("Read discrete inputs failed: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
    }
    return rc;
}

int ModbusSerialPort::writeSingleRegister(int addr, uint16_t value)
{
    if (!m_ctx || !m_isOpen) {
        m_lastError = QStringLiteral("Port not open");
        return -1;
    }
    int rc = modbus_write_register(m_ctx, addr, value);
    if (rc == -1) {
        m_lastError = QStringLiteral("Write single register failed: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
    }
    return rc;
}

int ModbusSerialPort::writeRegisters(int addr, int nb, const uint16_t *src)
{
    if (!m_ctx || !m_isOpen) {
        m_lastError = QStringLiteral("Port not open");
        return -1;
    }
    int rc = modbus_write_registers(m_ctx, addr, nb, src);
    if (rc == -1) {
        m_lastError = QStringLiteral("Write registers failed: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
    }
    return rc;
}

int ModbusSerialPort::writeSingleCoil(int addr, int status)
{
    if (!m_ctx || !m_isOpen) {
        m_lastError = QStringLiteral("Port not open");
        return -1;
    }
    int rc = modbus_write_bit(m_ctx, addr, status);
    if (rc == -1) {
        m_lastError = QStringLiteral("Write single coil failed: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
    }
    return rc;
}

int ModbusSerialPort::writeCoils(int addr, int nb, const uint8_t *src)
{
    if (!m_ctx || !m_isOpen) {
        m_lastError = QStringLiteral("Port not open");
        return -1;
    }
    int rc = modbus_write_bits(m_ctx, addr, nb, src);
    if (rc == -1) {
        m_lastError = QStringLiteral("Write coils failed: %1")
                          .arg(modbus_strerror(errno));
        emit errorOccurred(m_lastError);
    }
    return rc;
}

QString ModbusSerialPort::lastError() const
{
    return m_lastError;
}
