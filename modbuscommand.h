#ifndef MODBUSCOMMAND_H
#define MODBUSCOMMAND_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <cstdint>

class ModbusSerialPort;

///
/// \brief Modbus 指令交互类
///
/// 封装 Modbus RTU 通信指令的所有协议字段，支持通过 customField
/// 机制扩展自定义协议字段，方便后期添加业务层通信协议。
///
/// 使用示例:
/// \code
///   ModbusCommand cmd;
///   cmd.setSlaveAddress(1);
///   cmd.setFunctionCode(ModbusCommand::ReadHoldingRegisters);
///   cmd.setStartAddress(0x0000);
///   cmd.setQuantity(10);
///   if (cmd.execute(serialPort)) {
///       auto regs = cmd.responseRegisters();
///   }
///
///   // 扩展自定义协议字段
///   cmd.setCustomField("deviceType", 0x01);
///   cmd.setCustomField("channelId",  3);
/// \endcode
///
class ModbusCommand : public QObject
{
    Q_OBJECT

public:
    enum FunctionCode {
        ReadCoils              = 0x01,
        ReadDiscreteInputs     = 0x02,
        ReadHoldingRegisters   = 0x03,
        ReadInputRegisters     = 0x04,
        WriteSingleCoil        = 0x05,
        WriteSingleRegister    = 0x06,
        WriteMultipleCoils     = 0x0F,
        WriteMultipleRegisters = 0x10
    };
    Q_ENUM(FunctionCode)

    explicit ModbusCommand(QObject *parent = nullptr);

    // --- 标准 Modbus 协议字段 ---
    void setSlaveAddress(uint8_t address);
    uint8_t slaveAddress() const;

    void setFunctionCode(FunctionCode code);
    FunctionCode functionCode() const;

    void setStartAddress(uint16_t address);
    uint16_t startAddress() const;

    void setQuantity(uint16_t quantity);
    uint16_t quantity() const;

    void setWriteData(const QVector<uint16_t> &data);
    QVector<uint16_t> writeData() const;

    void setWriteBits(const QVector<uint8_t> &bits);
    QVector<uint8_t> writeBits() const;

    // --- 自定义扩展字段（方便后期添加通信协议字段）---
    void setCustomField(const QString &name, const QVariant &value);
    QVariant customField(const QString &name,
                         const QVariant &defaultValue = QVariant()) const;
    QStringList customFieldNames() const;

    // --- 执行指令 ---
    bool execute(ModbusSerialPort *port);

    // --- 响应数据 ---
    QVector<uint16_t> responseRegisters() const;
    QVector<uint8_t> responseCoils() const;
    QString lastError() const;

signals:
    void executed(bool success);

private:
    uint8_t m_slaveAddress = 1;
    FunctionCode m_functionCode = ReadHoldingRegisters;
    uint16_t m_startAddress = 0;
    uint16_t m_quantity = 1;

    QVector<uint16_t> m_writeData;
    QVector<uint8_t> m_writeBits;

    QVector<uint16_t> m_responseRegisters;
    QVector<uint8_t> m_responseCoils;

    QMap<QString, QVariant> m_customFields;
    QString m_lastError;
};

#endif // MODBUSCOMMAND_H
