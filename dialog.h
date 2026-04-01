/**
 * @file dialog.h
 * @brief Header for the Dialog class — serial port selection dialog.
 *
 * The Dialog provides a simple UI for users to view all available serial ports
 * and select one for connection. It enumerates ports at construction time and
 * emits the selected port name via the portSelected signal.
 *
 * @see MainWindow::on_btnSelectPort_clicked()
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}
class SerialDevice;

/**
 * @class Dialog
 * @brief Modal dialog for selecting a serial port from available system ports.
 *
 * At construction, this dialog:
 * 1. Creates its own SerialDevice instance to enumerate available ports
 * 2. Displays port names in a QLabel (for overview)
 * 3. Populates a QComboBox for selection
 *
 * When the user clicks "Select", the chosen port name is emitted via
 * portSelected and the dialog closes.
 *
 * ## UI Widgets
 * | Widget | Name | Purpose |
 * |--------|------|---------|
 * | QLabel | label_ports | Displays all available ports (newline-separated) |
 * | QComboBox | comboBox_selectPort | Dropdown for port selection |
 * | QPushButton | btnSelect | Confirms the selection |
 */
class Dialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the port selection dialog.
     *
     * Creates a SerialDevice to enumerate ports, populates the UI label
     * with all available port names, and fills the combo box for selection.
     *
     * @param parent Parent widget (typically MainWindow).
     */
    explicit Dialog(QWidget *parent = nullptr);

    /**
     * @brief Destroys the Dialog and frees UI resources.
     */
    ~Dialog();

signals:
    /**
     * @brief Emitted when the user confirms a port selection.
     *
     * Connected to MainWindow::onPortSelected() to trigger
     * a connection attempt to the chosen port.
     *
     * @param portName The system identifier of the selected port (e.g., "cu.usbmodem14101").
     */
    void portSelected(const QString &portName);

private slots:
    /**
     * @brief Handles the "Select" button click.
     *
     * Reads the current combo box selection, emits portSelected
     * with the chosen port name, and closes the dialog via accept().
     */
    void on_btnSelect_clicked();

private:
    Ui::Dialog *ui;            ///< Auto-generated UI pointer from dialog.ui
    SerialDevice *serialDevice; ///< Local SerialDevice instance used for port enumeration
};

#endif // DIALOG_H
