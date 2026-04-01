/**
 * @file dialog.cpp
 * @brief Implementation of the Dialog class — serial port selection dialog for WireFlux.
 *
 * Handles port enumeration at construction and emits the user's selection
 * back to MainWindow for connection.
 *
 * @see Dialog (class documentation in dialog.h)
 */

#include "dialog.h"
#include "ui_dialog.h"
#include "serialdevice.h"
#include <QDebug>

// ==========================================================
// Dialog Constructor
// ==========================================================

/**
 * @brief Initializes the port selection dialog and enumerates available ports.
 *
 * Setup sequence:
 * 1. Loads the UI from dialog.ui
 * 2. Creates a local SerialDevice for port discovery
 * 3. Queries available ports and displays them in label_ports (newline-separated)
 * 4. Iterates QSerialPortInfo directly to populate the combo box with port names
 *
 * @note The Dialog creates its own SerialDevice instance rather than sharing
 *       MainWindow's, since the Dialog is stack-allocated and short-lived.
 *
 * @param parent Parent widget (typically MainWindow).
 */
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , serialDevice(new SerialDevice(this))
{
    ui->setupUi(this);

    // Display all available ports as a newline-separated list
    QStringList ports = serialDevice->availablePorts();
    ui->label_ports->setText(ports.join("\n"));

    // Populate combo box from QSerialPortInfo (with debug logging)
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        qDebug() << "Found Port:" << info.portName();
        ui->comboBox_selectPort->addItem(info.portName());
    }

}

// ==========================================================
// Dialog Destructor
// ==========================================================

/**
 * @brief Destroys the Dialog and frees UI resources.
 */
Dialog::~Dialog()
{
    delete ui;
}

// ==========================================================
// Action: Confirm Port Selection
// ==========================================================

/**
 * @brief Processes the user's port selection.
 *
 * Reads the current text from comboBox_selectPort, emits the portSelected
 * signal with the chosen port name, and closes the dialog via accept().
 *
 * The emitted signal is received by MainWindow::onPortSelected(), which
 * stores the port name and immediately attempts a serial connection.
 */
void Dialog::on_btnSelect_clicked()
{
    QString selectedPort = ui->comboBox_selectPort->currentText();
    emit portSelected(selectedPort);
    accept(); // Close dialog with QDialog::Accepted result
}
