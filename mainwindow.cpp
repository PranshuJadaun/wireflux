/**
 * @file mainwindow.cpp
 * @brief Implementation of the MainWindow class — the core controller of WireFlux.
 *
 * This file contains all the logic for:
 * - Serial device connection management (connect/disconnect/port selection)
 * - Real-time chart setup and rendering (QtCharts with timer-driven updates)
 * - Data ingestion and buffering (normal mode + ECG 125 Hz mode)
 * - Peak detection with hysteresis for heart rate calculation
 * - ML buffer recording and cloud API submission for ECG classification
 * - Configuration dialog integration (baud rate, axis range, threshold)
 *
 * @see MainWindow, SerialDevice, Dialog, Config
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialdevice.h"
#include "dialog.h"
#include <QDebug>
#include "config.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

// ==========================================================
// MainWindow Constructor
// ==========================================================

/**
 * @brief Constructs the MainWindow and initializes all subsystems.
 *
 * Initialization sequence:
 * 1. Sets up the UI from mainwindow.ui
 * 2. Creates SerialDevice and wires signal-slot connections
 * 3. Hides the ECG group panel (shown only in ECG mode)
 * 4. Creates the QtCharts graph with dark theme styling
 * 5. Configures and starts the graph refresh timer
 * 6. Sets up the timer callback with plotting and axis update logic
 *
 * @param parent Optional parent widget.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serialDevice(new SerialDevice(this))
{
    ui->setupUi(this);
    this->setWindowTitle("WireFlux");

    // ==========================================================
    // Serial Device Signal-Slot Connections
    // Connect serial port events to the MainWindow's UI updates.
    // SerialDevice signals are connected to lambdas that update
    // the status bar and to the main data handler slot.
    // ==========================================================

    connect(serialDevice, &SerialDevice::deviceConnected,
            this, [this](const QString &port) {
                ui->statusbar->showMessage("Connected to " + port);
            });

    connect(serialDevice, &SerialDevice::errorOccurred,
            this, [this](const QString &error) {
                ui->statusbar->showMessage(error);
            });

    connect(serialDevice, &SerialDevice::dataReceived,
            this, &MainWindow::onDataReceived);

    connect(serialDevice, &SerialDevice::deviceDisconnected,
            this, [this]() {
                ui->statusbar->showMessage("Disconnected");
            });


    // Hide ECG-specific UI panel until ECG mode is explicitly enabled
    ui->ecgGroup->hide();

    // ==========================================================
    // Real-Time Graph Setup (QtCharts)
    // Creates a QChart with a single QLineSeries, dark-themed
    // background, and configurable X/Y axes.
    // ==========================================================

    // Initialize the line series that will hold all plotted data points
    series = new QLineSeries();
    
    // Create the chart container and configure axes
    QChart *chart = new QChart();
    axisX = new QValueAxis();
    axisY = new QValueAxis();
    
    // Attach the data series to the chart
    chart->addSeries(series);
    
    // Configure initial axis ranges and labels
    axisX->setRange(0, 10);
    axisY->setRange(0, 10);
    chart->setTitle("Value Chart");
    axisX->setTitleText("Time (s)");
    axisY->setTitleText("Value");
    
    // Bind axes to chart and attach them to the data series
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    // Apply dark theme aesthetics to the chart
    chart->legend()->hide();                                   // No legend needed for single series
    chart->setMargins(QMargins(0, 0, 0, 0));                  // Maximize chart area
    chart->setBackgroundRoundness(0);                          // Flat edges
    chart->setBackgroundBrush(QBrush(QColor("#121212")));      // Near-black background
    chart->setTitleBrush(QBrush(Qt::white));                   // White chart title text

    // Create the interactive chart view widget
    QChartView *chartView = new QChartView(chart);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setRenderHint(QPainter::Antialiasing); // Enable anti-aliasing for smooth line rendering

    // Insert the chart view into the UI's groupBox layout
    ui->groupBox->layout()->addWidget(chartView);

    // ==========================================================
    // Graph Refresh Timer Setup
    // The timer interval determines the plotting frequency:
    //   - Normal mode: 1000 ms (1 data point per second)
    //   - ECG mode:    10 ms   (~100 ticks/s, batch-plotting buffered 125 Hz data)
    // ==========================================================
    graphTimer = new QTimer(this);

    // Check initial ECG mode state from button text
    if(ui->ecgModeButton->text()=="ECG Mode : On"){
        graphTimer->start(10);       // Fast refresh for ECG data
        windowXSpan = 4.0;           // 4-second visible window
    }
    else{
        graphTimer->start(1000);     // Standard 1-second refresh
        windowXSpan = 10.0;          // 10-second visible window
    }


    // ──────────────────────────────────────────────────────────────────
    // Graph Timer Callback
    // This lambda executes on every timer tick to:
    //   1. Check if the device is connected (guard)
    //   2. Plot data from buffer (ECG/buffer mode) or latest value (normal)
    //   3. Trim old points to prevent memory growth
    //   4. Calculate paper-speed X-span in ECG mode
    //   5. Update X and Y axis ranges for the sliding window
    // ──────────────────────────────────────────────────────────────────
    connect(graphTimer, &QTimer::timeout, this, [this](){
        // Guard: only plot when actively connected ("Turn Off" means connection is live)
        if(ui->btnConnectionToggle->text() != "Turn Off") {
            return;
        }

        // ── Buffer Mode / ECG Mode Plotting ──────────────────────────
        // In buffer mode or ECG mode, ALL accumulated samples since the
        // last tick are plotted. Each sample is spaced 8ms apart (125 Hz).
        if (bufferMode || ui->ecgModeButton->text() == "ECG Mode : On") {
            if (dataBuffer.isEmpty()) {
                return; // Nothing to plot this tick
            }
            
            for (int bufVal : dataBuffer) {
                // 8ms per sample corresponds to the Arduino's ~125 Hz sample rate
                currentTime += 0.008; 
                series->append(currentTime, bufVal);
                
                // Dynamically expand Y-axis to accommodate data peaks
                if(bufVal > 0){
                    maxY = qMax(qMax(maxY, bufVal + 5), 10);
                } else {
                    maxY = qMax(qMax(maxY, (-1 * bufVal) + 5), 10);
                }
            }
            dataBuffer.clear(); // Consumed — ready for next batch
        } 
        // ── Normal (Single-Point) Plotting ───────────────────────────
        // When buffer mode is off in normal mode, only the most recent
        // sample is plotted. Time increments by exactly 1 second per tick.
        else {
            int valueToPlot = latestValue;
            currentTime += 1.0; // 1 tick = 1 second in normal mode
            series->append(currentTime, valueToPlot);
            
            // Dynamically expand Y-axis bounds
            if(valueToPlot > 0){
                maxY = qMax(qMax(maxY, valueToPlot + 5), 10);
            } else {
                maxY = qMax(qMax(maxY, (-1 * valueToPlot) + 5), 10);
            }
            dataBuffer.clear(); // Discard buffered data in non-buffer mode
        }

        // ── Memory Management: Trim Old Data Points ──────────────────
        // Limit stored chart points to prevent unbounded memory growth.
        // 5000 points is sufficient for ~40 seconds at 125 Hz or ~83 minutes at 1 Hz.
        const int maxPointsKept = 5000;
        if (series->count() > maxPointsKept) {
            series->removePoints(0, series->count() - maxPointsKept);
        }

        // ── Paper-Speed Calculation (ECG Mode Only) ──────────────────
        // In ECG mode, dynamically calculate the X-span so that
        // 1 second of data always occupies ~150 pixels on screen.
        // This mimics standard ECG paper speed (25 mm/s equivalent).
        if (ui->ecgModeButton->text() == "ECG Mode : On") {
            windowXSpan = (double)ui->groupBox->width() / 150.0;
        }

        // ── Axis Range Updates ───────────────────────────────────────
        // X-axis: sliding window showing the most recent `windowXSpan` seconds
        axisX->setRange(qMax(0.0, currentTime - windowXSpan), currentTime);

        // Y-axis: different scaling strategies for normal vs ECG mode
        if(ui->ecgModeButton->text()=="ECG Mode : On"){
            // Asymmetric range centered on centralR:
            // Lower bound = maxY - 2*(maxY - centralR) = 2*centralR - maxY
            axisY->setRange((maxY-2*(maxY-centralR)), maxY);
        }else{
            // Symmetric range centered on zero
            axisY->setRange(-1*maxY, maxY);
        }
    });
}



// ==========================================================
// MainWindow Destructor
// ==========================================================

/**
 * @brief Destroys the MainWindow and frees the auto-generated UI.
 *
 * Qt's parent-child ownership system handles cleanup of SerialDevice,
 * graphTimer, chart components, and other QObject children.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

// ==========================================================
// Action: Quick Start/Stop Serial Feed
// ==========================================================

/**
 * @brief Handles the connection toggle button click.
 *
 * Connection flow:
 * 1. If no port selected, auto-discovers and selects the first available port
 * 2. Attempts to open the serial connection at the configured baud rate
 * 3. Updates button text and status bar to reflect connection state
 *
 * Disconnection flow:
 * 1. Calls SerialDevice::disconnectPort()
 * 2. Reverts button text to "Turn On"
 */
void MainWindow::on_btnConnectionToggle_clicked()
{
    QString text = ui->btnConnectionToggle->text();

    // ── Starting Connection ──────────────────────────────────────
    if (text == "Turn On") {
        // Auto-select first available port if none has been explicitly chosen
        if (selectedPortName.isEmpty()) {
            QStringList ports = serialDevice->availablePorts();

            if (ports.isEmpty()) {
                ui->statusbar->showMessage("No serial ports found");
                return;
            }

            selectedPortName = ports.first();
            ui->label_port->setText("Port Selected : " + selectedPortName);
        }

        // Attempt physical serial connection
        if (!serialDevice->connectToPort(selectedPortName, BaudValue)) {
            ui->statusbar->showMessage("Failed to connect");
            return;
        }

        ui->btnConnectionToggle->setText("Turn Off");
    }
    // ── Ending Connection ────────────────────────────────────────
    else {
        serialDevice->disconnectPort();
        ui->btnConnectionToggle->setText("Turn On");
    }
}

// ==========================================================
// User selects a specific Port from the Dialog
// ==========================================================

/**
 * @brief Processes a port selection from the Dialog.
 *
 * Called when the user confirms a port in the selection dialog.
 * Immediately attempts to connect to the chosen port.
 *
 * @param portName System identifier of the selected port.
 */
void MainWindow::onPortSelected(const QString &portName)
{
    selectedPortName = portName;
    ui->label_port->setText("Port Selected : " + portName);

    // Immediately attempt connection to the newly selected port
    if (serialDevice->connectToPort(portName, BaudValue)) {
        ui->statusbar->showMessage("Connected to " + portName);
        ui->btnConnectionToggle->setText("Turn Off");
    } else {
        ui->statusbar->showMessage("Failed to connect");
        ui->btnConnectionToggle->setText("Turn On");
    }
}

// ==========================================================
// ECG API Communication
// ==========================================================

/**
 * @brief Sends a 200-sample ECG buffer to the cloud classification API.
 *
 * Constructs a JSON payload and performs an asynchronous HTTP POST.
 * The response is parsed for "result" (Normal/Abnormal) and
 * "confidence" (float) fields.
 *
 * ## Request
 * @code
 * POST https://ecgbackend.onrender.com/predict
 * Content-Type: application/json
 *
 * { "data": [542.0, 538.0, ...] }
 * @endcode
 *
 * ## Response
 * @code
 * { "result": "Normal", "confidence": 0.9842 }
 * @endcode
 *
 * @note Creates a new QNetworkAccessManager per invocation. Both the manager
 *       and reply are cleaned up via deleteLater() in the response handler.
 *
 * @param data Vector of exactly BUFFER_SIZE (200) float samples.
 */
void MainWindow::sendToAPI(const QVector<float>& data)
{
    // Set UI labels to "Waiting" while the request is in flight
    ui->ecgResult->setText("Waiting");
    ui->ecgConf->setText("Waiting");


    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QUrl url(apiUrl);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Convert QVector<float> to QJsonArray
    QJsonArray arr;
    for (float v : data)
        arr.append(v);

    QJsonObject obj;
    obj["data"] = arr;

    QJsonDocument doc(obj);

    qDebug() << "sendToAPI received data of size:" << data.size();
    qDebug() << "Payload being sent:" << doc.toJson(QJsonDocument::Compact);

    QNetworkReply *reply = manager->post(request, doc.toJson());

    // ── Asynchronous Response Handler ────────────────────────────
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qDebug() << "API Response:" << response;

            QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
            if (!jsonResponse.isNull() && jsonResponse.isObject()) {
                QJsonObject jsonObj = jsonResponse.object();

                QString result = jsonObj["result"].toString();
                double confidence = jsonObj["confidence"].toDouble();

                // Update result label with color coding
                ui->ecgResult->setText(result);

                if (result == "Abnormal") {
                    ui->ecgResult->setStyleSheet("color: red;");
                } else if (!result.isEmpty()) {
                    ui->ecgResult->setStyleSheet("color: green;");
                } else {
                    ui->ecgResult->setStyleSheet("");
                }

                ui->ecgConf->setText(QString::number(confidence));
            } else {
                // Malformed JSON response
                ui->ecgResult->setText("Invalid JSON");
                ui->ecgResult->setStyleSheet("color: red;");
                ui->ecgConf->setText("-");
            }
        } else {
            // Network-level error
            qDebug() << "API Error:" << reply->errorString();
            ui->ecgResult->setText("Network Error");
            ui->ecgResult->setStyleSheet("color: red;");
            ui->ecgConf->setText("-");
        }

        // Clean up allocated objects
        reply->deleteLater();
        manager->deleteLater();
    });
}
// ==========================================================
// Action: Open "Select Port" Dialog
// ==========================================================

/**
 * @brief Opens a modal dialog for serial port selection.
 *
 * The Dialog widget enumerates available ports and presents them
 * in a combo box. Upon confirmation, the portSelected signal is
 * routed to MainWindow::onPortSelected().
 *
 * The Dialog is stack-allocated so it is automatically destroyed
 * when this method returns (after the modal exec() completes).
 */
void MainWindow::on_btnSelectPort_clicked()
{
    Dialog sec(this);
    connect(&sec, &Dialog::portSelected,
            this, &MainWindow::onPortSelected);
    sec.exec(); // Blocks until user closes the dialog
}

// ==========================================================
// Event: Data Packet Received from Serial Device
// ==========================================================

/**
 * @brief Main data ingestion handler for incoming serial packets.
 *
 * This method is the central data processing hub, called for every
 * complete newline-delimited packet from the serial device.
 *
 * ## Processing Steps
 *
 * ### 1. Parsing
 * Converts the raw byte array to a trimmed string, then to an integer.
 * Non-numeric payloads are silently dropped with a debug log.
 *
 * ### 2. ML Buffer Management
 * - **Not recording**: Maintains a rolling window of the last BUFFER_SIZE
 *   (200) samples. Old samples are evicted from the front.
 * - **Recording**: Accumulates samples until BUFFER_SIZE is reached, then
 *   automatically stops recording and calls sendToAPI().
 *
 * ### 3. Graph Buffer
 * Appends the value to dataBuffer for consumption by the graph timer.
 *
 * ### 4. LCD Update
 * Displays the latest value on the primary QLCDNumber widget.
 *
 * ### 5. Peak Detection (Threshold + Hysteresis)
 * Detects R-peaks in ECG data using:
 * - Rising edge: value crosses above `threshold` while `rising == false`
 * - Falling edge: value drops below `threshold - 20` (hysteresis band)
 * - RR interval computed between consecutive peaks
 * - BPM calculated as `60000 / RR_ms` with a rolling 5-beat average
 * - Valid RR range: 300–2000 ms (30–200 BPM)
 *
 * @param data Raw byte array from SerialDevice containing a newline-terminated integer string.
 */
void MainWindow::onDataReceived(const QByteArray &data)
{
    // Capture current timestamp for RR interval calculation
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    // Parse: convert raw bytes to trimmed string, then to integer
    QString text = QString::fromUtf8(data).trimmed();

    if (text.isEmpty()) {
        return;
    }

    bool ok = false;
    int value = text.toInt(&ok);
    if (!ok) {
        qDebug() << "Ignoring non-numeric payload:" << text;
        return;
    }

    // Cache the latest value for normal-mode single-point plotting
    latestValue = value;


    // ── ML Buffer Logic ──────────────────────────────────────────
    // Two modes of operation based on isRecording flag:

    if (!isRecording) {
        // PASSIVE MODE: maintain a rolling window of last BUFFER_SIZE samples.
        // This ensures we always have recent data ready if the user starts recording.
        mlBuffer.append(value);
        while (mlBuffer.size() > BUFFER_SIZE) {
            mlBuffer.removeFirst(); // FIFO eviction of oldest sample
        }
    } else {
        // ACTIVE RECORDING: accumulate samples for API submission
        mlBuffer.append(value);

        if (mlBuffer.size() < BUFFER_SIZE) {
            // Update progress indicator
            ui->labelRecStatus->setText("Status : Recorded Part " + QString::number(mlBuffer.size()));
        } else if (mlBuffer.size() == BUFFER_SIZE) {
            // ── Buffer Complete: Prepare for API Submission ──────────
            // Display the last 10 samples for debugging/verification
            QString displayText = "";
            for (int i = 190; i < BUFFER_SIZE; i++) {
                if (i >= 0 && i < mlBuffer.size()) {
                    displayText += QString::number(mlBuffer[i]) + ", ";
                }
            }
            
            // Stop recording and update UI
            isRecording = false;
            ui->SampleData->setText(displayText);
            ui->ecgRec->setText("ECG Recording : Off");
            ui->labelRecStatus->setText("Status : ECG Recorded");
            
            // Dispatch 200-sample buffer to the cloud classification API
            sendToAPI(mlBuffer);
        } else {
            // Failsafe: buffer exceeded expected size (should not happen normally)
            isRecording = false;
        }
    }

    // Append to graph buffer for timer-driven plotting
    dataBuffer.push_back(latestValue);

    // Update the primary LCD readout
    ui->lcdNumber->display(latestValue);


    // ═════════════════════════════════════════════════════════════
    // Peak Detection Algorithm (Threshold + Hysteresis)
    //
    // Detects R-peaks in the ECG signal using a simple threshold
    // crossing approach with hysteresis to prevent double-counting
    // on noisy signals.
    //
    // State machine:
    //   rising=false → value > threshold → PEAK DETECTED, rising=true
    //   rising=true  → value < threshold-20 → rising=false (ready for next peak)
    // ═════════════════════════════════════════════════════════════

    if (value > threshold && !rising) {
        rising = true; // Mark ascending edge

        if (lastPeakTime != 0) {
            // Compute RR interval (time between consecutive R-peaks)
            int rr = currentTime - lastPeakTime;
            ui->lcdRR->display(rr);

            // Validate RR interval: 300ms (200 BPM) to 2000ms (30 BPM)
            if (rr > 300 && rr < 2000) {
                float bpm = 60000.0 / rr;

                // Maintain rolling history of last 5 BPM values
                bpmHistory.append(bpm);
                if (bpmHistory.size() > 5)
                    bpmHistory.removeFirst();

                // Compute and display rolling average BPM
                float avgBpm = 0;
                for (float b : bpmHistory)
                    avgBpm += b;
                avgBpm /= bpmHistory.size();

                ui->lcdBPM->display(avgBpm);
            }
        }

        lastPeakTime = currentTime;
    }

    // Hysteresis: only reset rising flag when signal drops significantly
    // below threshold (prevents re-triggering on noise/ringing)
    if (value < threshold - 20) {
        rising = false;
    }

}




// ==========================================================
// Action: Toggle Buffer Mode
// ==========================================================

/**
 * @brief Toggles buffer mode on or off with visual feedback.
 *
 * When buffer mode is enabled, the graph timer callback plots ALL
 * samples accumulated in dataBuffer since the last tick, rather than
 * only the latest single value. This is essential for high-frequency
 * data streams where a 1-second tick would otherwise drop samples.
 *
 * The button receives a cyan accent style when active.
 */
void MainWindow::on_btnBufferToggle_clicked()
{
    // Accent style for the "enabled" state
    const QString pressedStyle = "background-color: #00A8E8; color: white; border: 1px solid #0086B8;";

    if(ui->btnBufferToggle->text() == "Enable Buffer Mode"){
        bufferMode = true;
        ui->btnBufferToggle->setText("Disable Buffer Mode");
        ui->btnBufferToggle->setStyleSheet(pressedStyle); // Visual indicator of active state
    }
    else {
        bufferMode = false;
        ui->btnBufferToggle->setText("Enable Buffer Mode");
        ui->btnBufferToggle->setStyleSheet(""); // Revert to global stylesheet defaults
    }
}

// ==========================================================
// Action: Open Configuration Dialog
// ==========================================================

/**
 * @brief Opens the configuration dialog and wires up property change signals.
 *
 * Creates a Config dialog pre-populated with the current values of:
 * - Baud rate (BaudValue)
 * - Y-axis maximum (maxY)
 * - Central value (centralR)
 * - ECG threshold (threshold)
 *
 * Each Config signal is connected to a lambda that updates the corresponding
 * MainWindow member and (where applicable) immediately refreshes the chart axes.
 *
 * ## Central Value Guard
 * If the user sets centralR >= maxY, a QMessageBox::critical error is displayed
 * and the previous value is restored. This prevents an inverted or zero-height Y-axis.
 */
void MainWindow::on_actionSet_Config_triggered()
{
    Config conf(this);
    conf.setInitialValues(BaudValue, maxY, centralR, threshold);
    
    // ── Baud Rate Update ─────────────────────────────────────────
    connect(&conf, &Config::baudSent,
            this, [this](int data) {
                BaudValue = data; // Applied on next connectToPort() call
            });

    // ── Y-Axis Maximum Update ────────────────────────────────────
    connect(&conf, &Config::maxRSent,
            this, [this](int data) {
                maxY = data;
                axisY->setRange((maxY-2*(maxY-centralR)), maxY); // Immediately update chart
            });

    // ── Central Value Update (with safety guard) ─────────────────
    connect(&conf, &Config::centralSent,
            this, [this](int data) {
                int temp = centralR; // Backup for rollback
                centralR = data;
                if(maxY>centralR){
                    axisY->setRange((maxY-2*(maxY-centralR)), maxY);
                }else{
                    // Guard: centralR must be less than maxY
                    QMessageBox::critical(this, "Error", "Max Y value is less than central value. Central Value reverted to previous value");
                    centralR = temp; // Rollback
                }


            });

    // ── Threshold Update ─────────────────────────────────────────
    connect(&conf, &Config::thresholdSent,
            this, [this](int data) {
                    threshold = data;
                    ui->labelThreshold->setText("Threshold : " + QString::number(threshold));
            });
            
    // Block until user saves or closes the dialog
    conf.exec();
}

// ==========================================================
// Action: Toggle ECG Mode
// ==========================================================

/**
 * @brief Switches between normal monitoring mode and ECG mode.
 *
 * ## ECG Mode Activation
 * - Graph timer interval → 10 ms (from 1000 ms)
 * - X-axis span → 4 seconds initially (then dynamically adjusted by paper-speed logic)
 * - ECG group panel (BPM, RR, recording, API results) → visible
 * - Buffer mode is force-enabled in the timer callback
 *
 * ## ECG Mode Deactivation
 * - Graph timer interval → 1000 ms
 * - X-axis span → 10 seconds (fixed)
 * - ECG group panel → hidden
 */
void MainWindow::on_ecgModeButton_clicked()
{
    if(ui->ecgModeButton->text() == "ECG Mode : Off"){
        // ── Activate ECG Mode ────────────────────────────────────
        ui->ecgModeButton->setText("ECG Mode : On");
        graphTimer->start(10);           // 10 ms refresh rate
        windowXSpan = 4.0;              // Initial 4-second visible window
        ui->ecgGroup->show();           // Show ECG analysis panel
    }
    else{
        // ── Deactivate ECG Mode ──────────────────────────────────
        ui->ecgModeButton->setText("ECG Mode : Off");
        graphTimer->start(1000);         // Revert to 1-second refresh
        windowXSpan = 10.0;             // Standard 10-second window
        ui->ecgGroup->hide();           // Hide ECG analysis panel

    }

}


// ==========================================================
// Action: Toggle ECG Recording
// ==========================================================

/**
 * @brief Toggles ECG recording for ML API submission.
 *
 * When recording starts:
 * - The ML buffer is cleared to begin fresh capture
 * - isRecording is set to true (onDataReceived will accumulate samples)
 * - UI labels update to show active recording state
 *
 * When recording is manually stopped:
 * - isRecording is set to false
 * - Button text reverts to "ECG Recording : Off"
 *
 * @note Recording auto-stops when `mlBuffer.size() == BUFFER_SIZE` (200),
 *       triggering sendToAPI() from within onDataReceived().
 */
void MainWindow::on_ecgRec_clicked()
{

    if(!isRecording){
        // ── Start Recording ──────────────────────────────────────
        ui->ecgRec->setText("ECG Recording : On");
        ui->labelRecStatus->setText("Status : Recording started");
        mlBuffer.clear();              // Fresh buffer for new recording
        isRecording=true;
        isSaved = false;

    }else{
        // ── Stop Recording (manual) ──────────────────────────────
        ui->ecgRec->setText("ECG Recording : Off");
        isRecording=false;
    }
}
