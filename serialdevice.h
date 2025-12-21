#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>

class SerialDevice : public QObject
{
    Q_OBJECT

public:
    explicit SerialDevice(QObject *parent = nullptr);

    // Returns a list of available serial port names
    QStringList availablePorts() const;

    // Connect to a given serial port
    bool connectToPort(const QString &portName, int baudRate = 9600);

    // Disconnect from the serial port
    void disconnectPort();

signals:
    // Emitted when a device is successfully connected
    void deviceConnected(const QString &portName);

    // Emitted when the device is disconnected
    void deviceDisconnected();

    // Emitted when new data arrives from the device
    void dataReceived(const QByteArray &data);

    // Emitted when any serial error occurs
    void errorOccurred(const QString &error);

private slots:
    void handleReadyRead();


private:
    QSerialPort *serial;   // Handles the actual serial communication
};

#endif // SERIALDEVICE_H
