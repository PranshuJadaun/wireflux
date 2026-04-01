/**
 * @file config.h
 * @brief Header for the Config dialog — runtime configuration for serial and chart parameters.
 *
 * The Config dialog allows users to adjust:
 * - **Baud Rate**: Serial communication speed (dropdown with 7 standard options)
 * - **Max Data Value**: Y-axis upper bound for the chart
 * - **Central Value**: Reference center point for asymmetric Y-axis scaling in ECG mode
 * - **ECG Threshold**: R-peak detection threshold in ADC units
 *
 * Changes are communicated back to MainWindow via Qt signals emitted on save.
 * A "Revert to Default" button resets all values to compile-time constants.
 *
 * @see MainWindow::on_actionSet_Config_triggered()
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class Config;
}

/**
 * @class Config
 * @brief Modal configuration dialog for adjusting WireFlux runtime parameters.
 *
 * Provides a form-based UI for configuring serial communication and chart rendering
 * parameters. Values are bidirectionally synchronized with MainWindow:
 * - MainWindow passes current values via setInitialValues()
 * - Config emits signals on save to update MainWindow state
 *
 * ## Default Constants
 * | Parameter | Default | Description |
 * |-----------|---------|-------------|
 * | Baud Rate | 9600 | Standard serial speed |
 * | Max Range | 10 | Y-axis upper bound |
 * | Central | 0 | Y-axis center reference |
 * | Threshold | 580 | ECG peak detection threshold |
 */
class Config : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the Config dialog.
     *
     * Initializes the UI, populates the baud rate combo box with 7 standard values,
     * sets up input validation (QIntValidator for max range), and pre-fills
     * all fields with current default values.
     *
     * @param parent Parent widget (typically MainWindow).
     */
    explicit Config(QWidget *parent = nullptr);

    /**
     * @brief Destroys the Config dialog and frees UI resources.
     */
    ~Config();

    /**
     * @brief Pre-populates the dialog fields with values from MainWindow.
     *
     * Called before exec() to ensure the dialog reflects the current
     * application state rather than compile-time defaults.
     *
     * @param baud Current serial baud rate (selects matching combo box item).
     * @param maxRange Current Y-axis maximum value.
     * @param centralR Current central reference value for the chart.
     * @param threshold Current ECG peak detection threshold.
     */
    void setInitialValues(int baud, int maxRange, int centralR, int threshold);

private slots:
    /**
     * @brief Saves current dialog values and closes.
     *
     * Reads all UI fields, stores them in member variables,
     * emits all four signals (baudSent, maxRSent, centralSent, thresholdSent),
     * and calls accept() to close the dialog.
     */
    void on_btnSaveClose_clicked();

    /**
     * @brief Reverts all fields to compile-time default values.
     *
     * Resets:
     * - Baud rate → 9600
     * - Max range → 10
     * - Central value → 0
     * - Threshold → 580
     *
     * Updates both the UI widgets and emits all signals immediately.
     */
    void on_btnSetDefault_clicked();

signals:
    /**
     * @brief Emitted when the baud rate is saved.
     * @param baud The new baud rate value (e.g., 9600, 115200).
     */
    void baudSent(int baud);

    /**
     * @brief Emitted when the Y-axis maximum range is saved.
     * @param maxR The new maximum Y-axis value.
     */
    void maxRSent(int maxR);

    /**
     * @brief Emitted when the central reference value is saved.
     * @param centralR The new central value for asymmetric Y-axis scaling.
     */
    void centralSent(int centralR);

    /**
     * @brief Emitted when the ECG peak detection threshold is saved.
     * @param threshold The new threshold value in ADC units.
     */
    void thresholdSent(int threshold);

private:
    // ── Compile-Time Default Constants ───────────────────────────
    static constexpr int DefaultBaudRate = 9600;    ///< Default serial baud rate
    static constexpr int DefaultMaxRange = 10;      ///< Default Y-axis maximum
    static constexpr int DefaultCentral = 0;        ///< Default chart central value
    static constexpr int DefaultThreshold = 580;    ///< Default ECG peak detection threshold

    Ui::Config *ui;                                 ///< Auto-generated UI pointer from config.ui

    int baudRate = DefaultBaudRate;                  ///< Current baud rate value
    int maxR = DefaultMaxRange;                      ///< Current Y-axis maximum
    int centralR = DefaultCentral;                   ///< Current central reference value
    int threshold = DefaultThreshold;                ///< Current ECG threshold value

};

#endif // CONFIG_H
