#ifndef MODBUSSERIALPORT_H
#define MODBUSSERIALPORT_H

#include <QObject>
#include <QString>
#include <modbus.h>

class ModbusSerialPort : public QObject
{
    Q_OBJECT

public:
    explicit ModbusSerialPort(QObject *parent = nullptr);
    ~ModbusSerialPort();

    bool open(const QString &portName, int baudRate,
              char parity = 'N', int dataBits = 8, int stopBits = 1);
    void close();
    bool isOpen() const;

    void setSlaveAddress(int address);
    void setResponseTimeout(uint32_t milliseconds);

    int readHoldingRegisters(int addr, int nb, uint16_t *dest);
    int readInputRegisters(int addr, int nb, uint16_t *dest);
    int readCoils(int addr, int nb, uint8_t *dest);
    int readDiscreteInputs(int addr, int nb, uint8_t *dest);

    int writeSingleRegister(int addr, uint16_t value);
    int writeRegisters(int addr, int nb, const uint16_t *src);
    int writeSingleCoil(int addr, int status);
    int writeCoils(int addr, int nb, const uint8_t *src);

    QString lastError() const;

signals:
    void errorOccurred(const QString &error);
    void connectionStateChanged(bool connected);

private:
    modbus_t *m_ctx = nullptr;
    bool m_isOpen = false;
    QString m_lastError;
};

#endif // MODBUSSERIALPORT_H
