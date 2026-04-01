/**
 * @file mainwindow.h
 * @brief Header file for the MainWindow class — the central controller of the WireFlux application.
 *
 * MainWindow orchestrates:
 * - Serial port management via SerialDevice
 * - Real-time chart rendering via QtCharts (QLineSeries, QValueAxis, QChartView)
 * - ECG-specific features: peak detection, BPM calculation, recording, and API calls
 * - Configuration management through modal dialogs
 *
 * @see SerialDevice, Dialog, Config
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QObject>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QMessageBox>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class SerialDevice;

/**
 * @class MainWindow
 * @brief The primary application window managing serial I/O, real-time graphing, and ECG analysis.
 *
 * MainWindow is the core controller that ties together the serial communication layer
 * (SerialDevice), the live chart (QtCharts), modal configuration dialogs, and the
 * cloud-based ECG classification API.
 *
 * ## Operational Modes
 *
 * The application operates in two distinct modes:
 *
 * ### Normal Mode (Default)
 * - Graph timer fires every 1000 ms
 * - Each tick increments time by 1.0 second
 * - X-axis shows a 10-second sliding window
 * - Y-axis is symmetric: [-maxY, +maxY]
 * - Only the latest value is plotted unless buffer mode is enabled
 *
 * ### ECG Mode
 * - Graph timer fires every 10 ms
 * - Each buffered sample increments time by 0.008 s (125 Hz)
 * - X-axis span is dynamically calculated from widget width (paper-speed proportional)
 * - Y-axis is asymmetric around the configured central value
 * - Buffer mode is force-enabled (all samples plotted)
 * - ECG panel (BPM, RR, recording, API results) is visible
 *
 * ## Data Flow
 * @code
 * SerialDevice::dataReceived(QByteArray)
 *   → MainWindow::onDataReceived()
 *     → parse integer
 *     → update latestValue, dataBuffer, mlBuffer, LCD
 *     → run peak detection (threshold + hysteresis)
 *     → if recording complete: sendToAPI()
 *
 * QTimer::timeout()
 *   → plot dataBuffer or latestValue to QLineSeries
 *   → update axis ranges
 *   → trim old points
 * @endcode
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the MainWindow.
     *
     * Initializes the UI from mainwindow.ui, creates the SerialDevice instance,
     * wires up all signal-slot connections, configures the QtCharts graph
     * (series, axes, chart view), and starts the graph refresh timer.
     *
     * @param parent Optional parent widget (defaults to nullptr for top-level window).
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Sends a 200-sample ECG data buffer to the cloud classification API.
     *
     * Creates a JSON payload `{ "data": [v1, v2, ..., v200] }` and POSTs it to
     * the ECG backend API. On success, parses the "result" (Normal/Abnormal) and
     * "confidence" fields to update the UI labels. On failure, displays error text.
     *
     * @note Allocates a new QNetworkAccessManager per call (cleaned up via deleteLater).
     *
     * @param data Vector of 200 float samples to classify.
     */
    void sendToAPI(const QVector<float>& data);

    /**
     * @brief Destroys the MainWindow and frees UI resources.
     */
    ~MainWindow();

private slots:
    /**
     * @brief Toggles the serial connection on or off.
     *
     * When "Turn On" is clicked:
     * - If no port is selected, auto-selects the first available port
     * - Attempts connection via SerialDevice::connectToPort()
     * - On success, changes button text to "Turn Off"
     *
     * When "Turn Off" is clicked:
     * - Disconnects the serial port
     * - Reverts button text to "Turn On"
     */
    void on_btnConnectionToggle_clicked();

    /**
     * @brief Opens the port selection dialog.
     *
     * Instantiates a Dialog on the stack, connects its portSelected signal
     * to MainWindow::onPortSelected, and executes the dialog modally.
     */
    void on_btnSelectPort_clicked();

    /**
     * @brief Handles user port selection from the Dialog.
     *
     * Stores the selected port name, updates the port label in the UI,
     * and immediately attempts to connect to the new port.
     *
     * @param portName The name of the selected serial port (e.g., "cu.usbmodem14101").
     */
    void onPortSelected(const QString &portName);

    /**
     * @brief Processes incoming serial data packets.
     *
     * This is the main data ingestion handler, called for every complete
     * newline-delimited packet from the serial device. It performs:
     *
     * 1. **Parsing**: Trims whitespace, converts to integer (drops non-numeric)
     * 2. **State update**: Sets latestValue, appends to dataBuffer
     * 3. **LCD refresh**: Updates the primary LCD readout
     * 4. **ML buffer management**:
     *    - When NOT recording: maintains a rolling 200-sample window
     *    - When recording: accumulates until 200 samples, then auto-submits to API
     * 5. **Peak detection**: Threshold-with-hysteresis R-peak detection
     *    - Computes RR interval and rolling 5-beat average BPM
     *    - Valid range: 300–2000 ms (30–200 BPM)
     *
     * @param data Raw byte array from SerialDevice (e.g., "542\n").
     */
    void onDataReceived(const QByteArray &data);

    /**
     * @brief Toggles buffer mode on or off.
     *
     * When enabled, the graph timer plots ALL buffered samples each tick
     * instead of only the latest value. The button gets an accent style
     * (`#00A8E8` background) to indicate the active state.
     */
    void on_btnBufferToggle_clicked();

    /**
     * @brief Opens the configuration dialog (Menu → Set Config).
     *
     * Creates a Config dialog pre-populated with current values, connects
     * its output signals (baudSent, maxRSent, centralSent, thresholdSent)
     * to lambdas that update MainWindow's internal state and chart axes.
     *
     * The central value is guarded: if `centralR >= maxY`, a critical error
     * message is shown and the change is reverted.
     */
    void on_actionSet_Config_triggered();

    /**
     * @brief Toggles ECG mode on or off.
     *
     * ECG Mode On:
     * - Switches graph timer to 10 ms interval
     * - Sets X-span to 4 seconds initially (then dynamically adjusted)
     * - Shows the ECG group panel (BPM, RR, recording controls, API results)
     *
     * ECG Mode Off:
     * - Reverts graph timer to 1000 ms interval
     * - Sets X-span back to 10 seconds
     * - Hides the ECG group panel
     */
    void on_ecgModeButton_clicked();

    /**
     * @brief Toggles ECG recording for ML API submission.
     *
     * When recording starts:
     * - Clears the ML buffer
     * - Sets isRecording = true
     * - Updates button and status labels
     *
     * When recording is manually stopped:
     * - Sets isRecording = false
     * - Updates button text
     *
     * Recording auto-stops when mlBuffer reaches BUFFER_SIZE (200 samples),
     * at which point sendToAPI() is called automatically.
     */
    void on_ecgRec_clicked();

private:
    Ui::MainWindow *ui;                 ///< Auto-generated UI pointer from mainwindow.ui
    SerialDevice *serialDevice;         ///< Manages the underlying QSerialPort connection
    
    // ── Graph State ──────────────────────────────────────────────────
    int latestValue = 0;                ///< Most recently received integer sample value
    QLineSeries *series;                ///< The line series holding all plotted data points
    QValueAxis *axisX;                  ///< X-axis representing elapsed time in seconds
    QValueAxis *axisY;                  ///< Y-axis representing sample amplitude
    QTimer *graphTimer;                 ///< Timer controlling graph refresh rate (10ms or 1000ms)
    
    double currentTime = 0.0;          ///< Virtual elapsed time (seconds) for X-axis positioning
    int maxY = 10;                      ///< Dynamic Y-axis upper bound (auto-expands with data)
    int centralR = 0;                   ///< Central reference value for asymmetric Y-axis in ECG mode
    double windowXSpan = 10.0;          ///< Visible X-axis window width in seconds (10s normal, dynamic ECG)
    QVector<int> dataBuffer;            ///< Temporary buffer of samples received between graph timer ticks
    bool bufferMode = false;            ///< Flag: true if buffer plotting is active (manual or ECG forced)

    // ── ML / ECG Recording ───────────────────────────────────────────
    QVector<float> mlBuffer;            ///< Rolling or recording buffer of samples for API submission
    const int BUFFER_SIZE = 200;        ///< Number of samples required for one ML API prediction
    bool isRecording = false;           ///< Flag: true while actively recording for API submission
    bool isSaved = false;               ///< Flag: tracking whether the recorded data has been saved

    // ── Serial Configuration ─────────────────────────────────────────
    int BaudValue = 9600;               ///< Active baud rate for serial connections (configurable via Config)
    QString selectedPortName;           ///< Currently selected serial port identifier (e.g., "cu.usbmodem14101")
    QString text = "";                  ///< Unused legacy text variable

    // ── API Configuration ────────────────────────────────────────────
    QString apiUrl = "https://ecgbackend.onrender.com/predict"; ///< ECG classification POST endpoint (expects 200 samples)
    QString healthUrl = "https://ecgbackend.onrender.com/";     ///< API health check GET endpoint
    bool isApiBusy = false;             ///< Guard flag to prevent concurrent API calls (currently unused)

    // ── Peak Detection ───────────────────────────────────────────────
    int threshold = 580;                ///< R-peak detection threshold in ADC units (configurable)
    QNetworkAccessManager *manager;     ///< HTTP client for API calls (member variable, but sendToAPI creates its own)
    QJsonArray dataArray;               ///< Unused JSON array (legacy)

    bool rising = false;                ///< Peak detection state: true when signal is above threshold (ascending edge)
    qint64 lastPeakTime = 0;            ///< Timestamp (ms since epoch) of the last detected R-peak

    QVector<float> bpmHistory;          ///< Rolling window of last 5 BPM values for averaging


};
#endif // MAINWINDOW_H
