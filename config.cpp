/**
 * @file config.cpp
 * @brief Implementation of the Config dialog — runtime configuration for WireFlux.
 *
 * Handles the configuration form UI logic including:
 * - Populating baud rate options (7 standard values)
 * - Input validation for numeric fields
 * - Save and emit workflow
 * - Revert to defaults functionality
 * - Initial value loading from MainWindow state
 *
 * @see Config (class documentation in config.h)
 */

#include "config.h"
#include "ui_config.h"
#include <QPixmap>
#include <QIntValidator>

// ==========================================================
// Config Constructor
// ==========================================================

/**
 * @brief Initializes the configuration dialog.
 *
 * Setup sequence:
 * 1. Loads the UI from config.ui
 * 2. Populates the baud rate combo box with 7 standard values
 * 3. Sets a QIntValidator on the max range field (1–1,000,000)
 * 4. Pre-fills text fields with current default values
 * 5. Selects the matching baud rate in the combo box
 *
 * @param parent Parent widget (typically MainWindow).
 */
Config::Config(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Config)
{
    ui->setupUi(this);
    ui->label_Version->setText(APP_VERSION);

    // Populate baud rate combo box with standard serial speeds
    ui->comboBox_BaudList->addItem("9600");
    ui->comboBox_BaudList->addItem("14400");
    ui->comboBox_BaudList->addItem("19200");
    ui->comboBox_BaudList->addItem("28800");
    ui->comboBox_BaudList->addItem("38400");
    ui->comboBox_BaudList->addItem("57600");
    ui->comboBox_BaudList->addItem("115200");

    // Apply input validation: max range must be an integer between 1 and 1,000,000
    ui->lineEdit_Max->setValidator(new QIntValidator(1, 1000000, this));
    ui->lineEdit_Max->setText(QString::number(maxR));
    ui->lineEdit_Central->setText(QString::number(centralR));

    // Select the current baud rate in the combo box
    const int defaultIndex = ui->comboBox_BaudList->findText(QString::number(baudRate));
    if (defaultIndex >= 0) {
        ui->comboBox_BaudList->setCurrentIndex(defaultIndex);
    }
}

// ==========================================================
// Config Destructor
// ==========================================================

/**
 * @brief Destroys the Config dialog and frees UI resources.
 */
Config::~Config()
{
    delete ui;
}

// ==========================================================
// Action: Save and Close
// ==========================================================

/**
 * @brief Reads all dialog fields, emits signals, and closes.
 *
 * Workflow:
 * 1. Parse baud rate from combo box selection
 * 2. Parse max range from line edit
 * 3. Parse central value from line edit
 * 4. Parse threshold from line edit
 * 5. Emit all four signals so MainWindow can update its state
 * 6. Call accept() to close the dialog with QDialog::Accepted result
 */
void Config::on_btnSaveClose_clicked()
{
    baudRate = ui->comboBox_BaudList->currentText().toInt();
    maxR = ui->lineEdit_Max->text().toInt();
    centralR = ui->lineEdit_Central->text().toInt();
    threshold = ui->lineEdit_threshold->text().toInt();

    emit baudSent(baudRate);
    emit maxRSent(maxR);
    emit centralSent(centralR);
    emit thresholdSent(threshold);

    accept();  // Close the dialog
}

// ==========================================================
// Action: Revert to Defaults
// ==========================================================

/**
 * @brief Resets all configuration fields to compile-time default values.
 *
 * Restores:
 * - Baud rate → 9600
 * - Max range → 10
 * - Central value → 0
 * - Threshold → 580
 *
 * Updates both the internal member variables and the UI widgets,
 * then emits all signals immediately so MainWindow gets the
 * default values in real-time without requiring a "Save".
 */
void Config::on_btnSetDefault_clicked()
{
    // Reset internal state to defaults
    baudRate = DefaultBaudRate;
    maxR = DefaultMaxRange;
    centralR = DefaultCentral;
    threshold = DefaultThreshold;

    // Update baud rate combo box selection
    const int defaultIndex = ui->comboBox_BaudList->findText(QString::number(DefaultBaudRate));
    if (defaultIndex >= 0) {
        ui->comboBox_BaudList->setCurrentIndex(defaultIndex);
    }

    // Update text fields
    ui->lineEdit_Max->setText(QString::number(DefaultMaxRange));
    ui->lineEdit_Central->setText(QString::number(DefaultCentral));

    // Emit all signals to immediately propagate defaults to MainWindow
    emit baudSent(DefaultBaudRate);
    emit maxRSent(DefaultMaxRange);
    emit centralSent(DefaultCentral);
    emit thresholdSent(threshold);


}

// ==========================================================
// Setter: Pre-populate from MainWindow State
// ==========================================================

/**
 * @brief Loads current MainWindow values into the dialog fields.
 *
 * Called by MainWindow::on_actionSet_Config_triggered() before
 * executing the dialog, ensuring the form reflects the active
 * application state rather than compile-time defaults.
 *
 * @param baud Current baud rate from MainWindow (matched in combo box).
 * @param maxRange Current Y-axis maximum from MainWindow.
 * @param centralR Current central value from MainWindow.
 * @param threshold Current ECG threshold from MainWindow.
 */
void Config::setInitialValues(int baud, int maxRange,int centralR,int threshold)
{
    // Update internal members (only if valid)
    if (baud > 0) {
        baudRate = baud;
    }
    if (maxRange > 0) {
        maxR = maxRange;
    }

    // Sync combo box to match the provided baud rate
    const int index = ui->comboBox_BaudList->findText(QString::number(baudRate));
    if (index >= 0) {
        ui->comboBox_BaudList->setCurrentIndex(index);
    }

    // Sync all text fields
    ui->lineEdit_Max->setText(QString::number(maxR));
    ui->lineEdit_Central->setText(QString::number(centralR));
    ui->lineEdit_threshold->setText(QString::number(threshold));


}
