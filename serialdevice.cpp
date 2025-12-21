#include "serialdevice.h"

// Constructor
SerialDevice::SerialDevice(QObject *parent)
    : QObject(parent),
    serial(new QSerialPort(this))
{
    connect(serial, &QSerialPort::readyRead,
            this, &SerialDevice::handleReadyRead);
}

// List available ports
QStringList SerialDevice::availablePorts() const
{
    QStringList portList;

    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        portList << port.portName();
    }

    return portList;
}

// Connect to a port
bool SerialDevice::connectToPort(const QString &portName, int baudRate)
{
    if (serial->isOpen()) {
        serial->close();
    }

    serial->setPortName(portName);
    serial->setBaudRate(baudRate);

    if (!serial->open(QIODevice::ReadOnly)) {
        emit errorOccurred(serial->errorString());
        return false;
    }

    emit deviceConnected(portName);
    return true;
}

// Read incoming data
void SerialDevice::handleReadyRead()
{
    QByteArray data = serial->readAll();

    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

// Disconnect port
void SerialDevice::disconnectPort()
{
    if (serial->isOpen()) {
        serial->close();
        emit deviceDisconnected();
    }
}
