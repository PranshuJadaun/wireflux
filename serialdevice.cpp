/**
 * @file serialdevice.cpp
 * @brief Implementation of the SerialDevice class — serial port abstraction for WireFlux.
 *
 * Handles all low-level serial communication including port discovery,
 * connection/disconnection lifecycle, and data packet reconstruction
 * from fragmented serial reads.
 *
 * @see SerialDevice (class documentation in serialdevice.h)
 */

#include "serialdevice.h"

// ==========================================================
// SerialDevice Constructor
// ==========================================================

/**
 * @brief Initializes the serial device manager.
 *
 * Creates the internal QSerialPort and connects its readyRead signal
 * to the handleReadyRead slot. The readyRead signal fires whenever
 * the OS kernel serial buffer has new data available for reading.
 *
 * @param parent Parent QObject for memory management.
 */
SerialDevice::SerialDevice(QObject *parent)
    : QObject(parent),
    serial(new QSerialPort(this))
{
    // Bind the hardware-level readyRead signal to our internal data parsing slot
    connect(serial, &QSerialPort::readyRead,
            this, &SerialDevice::handleReadyRead);
}

// ==========================================================
// Query System Available Ports
// ==========================================================

/**
 * @brief Enumerates all serial ports currently recognized by the operating system.
 *
 * Uses QSerialPortInfo to query the OS for connected serial interfaces.
 * On macOS, this typically returns entries like "cu.usbmodem14101" or
 * "cu.Bluetooth-Incoming-Port". On Windows, entries like "COM3".
 *
 * @return QStringList of port names. Empty list if no ports are detected.
 */
QStringList SerialDevice::availablePorts() const
{
    QStringList portList;

    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        portList << port.portName();
    }

    return portList;
}

// ==========================================================
// Attempt Device Connection
// ==========================================================

/**
 * @brief Opens a serial port connection with the specified parameters.
 *
 * ## Connection Protocol
 * 1. If a port is already open, close it first (prevents resource conflicts)
 * 2. Set the target port name and baud rate
 * 3. Attempt to open in ReadOnly mode (write support not currently needed)
 * 4. Emit deviceConnected on success, or errorOccurred on failure
 *
 * @param portName System identifier for the target port.
 * @param baudRate Communication speed (default 9600 bps).
 * @return true on successful connection, false on failure.
 */
bool SerialDevice::connectToPort(const QString &portName, int baudRate)
{
    // Safely close any existing connection to free system resources
    if (serial->isOpen()) {
        serial->close();
    }

    // Configure the target serial parameters
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);

    // Attempt to acquire the device port (ReadOnly — no write operations)
    if (!serial->open(QIODevice::ReadOnly)) {
        emit errorOccurred(serial->errorString()); // Broadcast failure details to UI
        return false;
    }

    emit deviceConnected(portName); // Notify UI of successful connection
    return true;
}

// ==========================================================
// Data Reading & Reconstruction Algorithm
// ==========================================================

/**
 * @brief Reconstructs complete data lines from potentially fragmented serial reads.
 *
 * Serial communication often delivers data in unpredictable chunks:
 * a single readyRead signal might deliver "5", then "42\n", or "542\n"
 * all at once. This method uses `canReadLine()` to guarantee we only
 * process complete newline-terminated lines, preventing parseInt failures
 * on partial data.
 *
 * Each complete line is emitted via the dataReceived signal for processing
 * by MainWindow::onDataReceived().
 */
void SerialDevice::handleReadyRead()
{
    // Process all available complete lines (there may be multiple)
    while (serial->canReadLine()) {
        QByteArray data = serial->readLine();
        if (!data.isEmpty()) {
            emit dataReceived(data); // Forward the complete packet to listeners
        }
    }
}

// ==========================================================
// Device Disconnection Protocol
// ==========================================================

/**
 * @brief Safely closes the active serial port connection.
 *
 * Checks that the port is actually open before attempting to close,
 * preventing errors on redundant disconnect calls. Emits
 * deviceDisconnected to notify the UI to reset its connection state.
 */
void SerialDevice::disconnectPort()
{
    if (serial->isOpen()) {
        serial->close();
        emit deviceDisconnected(); // Alert the UI to reset button states
    }
}
