/**
 * @file serialdevice.h
 * @brief Header for the SerialDevice class — a QSerialPort abstraction layer.
 *
 * SerialDevice encapsulates all serial port management, providing a clean interface
 * for discovering available ports, establishing/closing connections, and emitting
 * reconstructed data packets to the application.
 *
 * ## Protocol
 * SerialDevice expects **newline-delimited** data from the connected device.
 * Each complete line (terminated by `\n`) is emitted as a separate dataReceived signal.
 * Fragmented reads are internally buffered by QSerialPort::canReadLine().
 *
 * ## Ownership
 * The internal QSerialPort is owned by this class (created with `this` as parent).
 * When SerialDevice is destroyed, the port is automatically closed and freed.
 *
 * @see MainWindow (primary consumer of SerialDevice signals)
 */

#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>

/**
 * @class SerialDevice
 * @brief Manages serial port interactions for the WireFlux application.
 *
 * This class wraps QSerialPort to provide:
 * - **Port Discovery**: Enumerate all available serial ports via the OS
 * - **Connection Management**: Open/close ports with configurable baud rate
 * - **Data Reconstruction**: Buffer incoming bytes and emit complete lines
 * - **Event Notifications**: Signals for connection state changes and errors
 *
 * ## Usage Example
 * @code
 * SerialDevice *device = new SerialDevice(this);
 *
 * connect(device, &SerialDevice::dataReceived, this, [](const QByteArray &data) {
 *     int value = QString::fromUtf8(data).trimmed().toInt();
 *     // Process value...
 * });
 *
 * QStringList ports = device->availablePorts();
 * if (!ports.isEmpty()) {
 *     device->connectToPort(ports.first(), 9600);
 * }
 * @endcode
 */
class SerialDevice : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the SerialDevice and initializes the internal QSerialPort.
     *
     * Creates a QSerialPort instance and connects its readyRead signal to the
     * internal handleReadyRead slot for automatic data processing.
     *
     * @param parent The parent QObject, taking ownership of this instance.
     */
    explicit SerialDevice(QObject *parent = nullptr);

    /**
     * @brief Enumerates all available serial ports on the system.
     *
     * Queries the OS via QSerialPortInfo::availablePorts() and returns
     * the port names as a string list (e.g., ["cu.usbmodem14101", "cu.Bluetooth-Incoming-Port"]).
     *
     * @return QStringList of available port names. Empty if no ports found.
     */
    QStringList availablePorts() const;

    /**
     * @brief Attempts to connect to a specific serial port.
     *
     * If the device is already connected to another port, the existing connection
     * is closed first. The port is opened in **ReadOnly** mode.
     *
     * @param portName The system identifier for the port (e.g., "COM3", "cu.usbmodem14101").
     * @param baudRate The communication speed in bits per second (default: 9600).
     *                 Common values: 9600, 14400, 19200, 28800, 38400, 57600, 115200.
     * @return true if the connection was successfully established.
     * @return false if the port could not be opened (emits errorOccurred with details).
     */
    bool connectToPort(const QString &portName, int baudRate = 9600);

    /**
     * @brief Disconnects from the current serial port.
     *
     * Closes the serial port if it is currently open and emits deviceDisconnected.
     * Safe to call even if no port is connected (no-op in that case).
     */
    void disconnectPort();

signals:
    /**
     * @brief Emitted when a serial port connection is successfully established.
     * @param portName The name of the port that was connected to.
     */
    void deviceConnected(const QString &portName);

    /**
     * @brief Emitted when the active serial port connection is closed.
     */
    void deviceDisconnected();

    /**
     * @brief Emitted when a complete newline-delimited data packet is received.
     *
     * Each emission represents one complete line from the serial device.
     * The data includes the newline character; callers should trim as needed.
     *
     * @param data The raw byte array of the complete line.
     */
    void dataReceived(const QByteArray &data);

    /**
     * @brief Emitted when a serial port error occurs (e.g., failed to open).
     * @param error A human-readable description of the error from QSerialPort::errorString().
     */
    void errorOccurred(const QString &error);

private slots:
    /**
     * @brief Internal handler for QSerialPort::readyRead signal.
     *
     * Called by the OS whenever new bytes arrive on the serial bus.
     * Uses canReadLine() to ensure only complete newline-terminated lines
     * are processed, preventing partial-data parsing issues.
     *
     * Algorithm:
     * @code
     * while (serial->canReadLine()):
     *     line = serial->readLine()
     *     if (line is not empty):
     *         emit dataReceived(line)
     * @endcode
     */
    void handleReadyRead();

private:
    QSerialPort *serial; ///< Internal Qt serial port object managing low-level I/O
};

#endif // SERIALDEVICE_H
