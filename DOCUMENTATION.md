# WireFlux Serial Monitor Documentation

## Overview
WireFlux is a Qt 6 desktop application for monitoring serial data streams (e.g., Arduino/ESP devices) and plotting incoming numeric samples on a live chart. The UI exposes serial connection controls, buffer-mode visualization, configuration options, and an LCD readout of the latest value.

## Build & Runtime Requirements
- **CMake**: 3.19+
- **Qt**: 6.5+ with modules `Qt6::Core`, `Qt6::Widgets`, `Qt6::SerialPort`, `Qt6::Charts`
- **Compiler**: C++17-capable toolchain supported by Qt
- **Resources**: `resources.qrc` embeds `:/images/E.png`

### Build Steps
1. Configure the project with CMake, ensuring Qt 6.5+ is discoverable on the system:
   ```bash
   cmake -S . -B build -DCMAKE_PREFIX_PATH="/path/to/Qt/6.5.0/macos" 
   ```
2. Build the generated project:
   ```bash
   cmake --build build
   ```
3. Run the resulting executable (`untitled` bundle on macOS). On first run, connect a serial device that streams newline-separated integers.

## Application Flow
```mermaid
flowchart LR
    main[main.cpp] --> mw[MainWindow]
    mw -->|menu action| cfg[Config Dialog]
    mw -->|Select Port| dlg[Port Dialog]
    dlg -->|portSelected(QString)| mw
    mw -->|connectToPort| ser[SerialDevice]
    ser -->|dataReceived(QByteArray)| mw
    mw --> chart[QChartView]
```

## Modules
- **`main.cpp`**: Initializes `QApplication`, instantiates `MainWindow`, and runs the event loop.
- **`SerialDevice` (`serialdevice.h/.cpp`)**:
  - Wraps `QSerialPort` for discovery (`availablePorts()`), connection (`connectToPort()`), disconnection, and read handling.
  - Emits signals `deviceConnected`, `deviceDisconnected`, `dataReceived`, and `errorOccurred`.
- **`Dialog` (`dialog.h/.cpp` & `dialog.ui`)**:
  - Lists available serial ports and allows the user to select one.
  - Filters macOS-style ports (`"cu.usb"`) for quick selection and emits `portSelected` when confirmed.
- **`Config` (`config.h/.cpp` & `config.ui`)**:
  - Provides UI to set baud rate and Y-axis range.
  - Validates numeric max input and offers a “Revert to Default” action (`9600` baud, ±10 max range).
  - Emits `baudSent` and `maxRSent` when the user saves changes.
- **`MainWindow` (`mainwindow.h/.cpp` & `mainwindow.ui`)**:
  - Hosts the central UI, manages serial connections, handles received data, and drives chart updates.
  - Connects buttons and menu actions to slots for selecting ports, toggling data streaming, enabling buffer mode, and opening configuration.
  - Maintains chart state via `QLineSeries`, `QValueAxis`, and a `QTimer` for periodic plotting.

## User Interface Details
- **Select Port (`pushButton`)**: Opens `Dialog`. When a port is chosen, `MainWindow::onPortSelected()` persists the selection, attempts connection, updates status bar, and toggles the Start button.
- **Start/Stop (`pushButton_start`)**:
  - “Turn On”: Connects using the selected port (or selects the first discovered port) with the configured baud rate.
  - “Turn Off”: Disconnects the serial device.
- **Buffer Mode (`pushButton_2`)**: Toggles buffered plotting. When enabled, the chart plots the most recent buffered sample each timer tick; when disabled, it plots the latest immediate value.
- **LCD Display (`lcdNumber`)**: Shows the latest parsed numeric value.
- **Status Bar**: Reflects connection status, errors, and disconnection notices.
- **Menu > Set Config**: Launches `Config`. Saved values adjust the baud rate for future connects and update the chart’s Y-axis immediately.

## Serial Data Handling
- `SerialDevice::handleReadyRead()` reads all available bytes and emits `dataReceived` for non-empty payloads.
- `MainWindow::onDataReceived()`
  - Trims incoming text and validates it as an integer (ignoring malformed frames).
  - Updates `latestValue`, pushes to the buffer, and refreshes the LCD display.
- The timer callback (`QTimer::timeout`) executes every 1000 ms:
  - Selects `valueToPlot` from the buffer (if buffer mode) or the latest value.
  - Appends the value to the `QLineSeries`, trimming the oldest points beyond 600 samples to limit memory usage.
  - Scrolls the X-axis window to the last 10 seconds, adapts Y-axis bounds symmetrically around zero, and clears the buffer when consumed.

## Configuration Workflow
- `MainWindow::on_actionSet_Config_triggered()` creates `Config`, calls `setInitialValues()` to reflect current baud and max range, and connects dialog signals to update internal state.
- `Config::on_pushButton_setDefault_clicked()` reverts to default baud (`9600`) and max range (`10`), synchronizing UI widgets immediately.
- When the user saves, the `Config` dialog emits `baudSent` and `maxRSent`, driving real-time updates in `MainWindow` (including adjusting `axisY`).

## Charting Behavior
- Chart created in `MainWindow` uses dark theme styling and hides the legend.
- X-axis represents elapsed seconds (`timeCounter` increments once per tick) with a sliding window.
- Y-axis expands automatically to include new values (with a ±5 margin) but can also be manually constrained via the Config dialog.
- Anti-aliasing is enabled for smooth visuals.

## Buffer Mode Logic
- `bufferMode` flag controls whether the timer plots the latest buffered sample or the last immediate value.
- Buffer mode aids in smoothing bursts: data points accumulate in `dataBuffer`, and the most recent one is plotted on each tick while the buffer remains populated.
- Disabling buffer mode restores immediate plotting and clears the button stylesheet.

## Resource Management & Memory Notes
- `SerialDevice` owns `QSerialPort` and closes it safely before reconnecting.
- `MainWindow` limits chart history to 600 points; adjust `maxPointsKept` if longer history is required.
- `Config` validators protect against invalid numeric input.
- Qt parent-child ownership ensures dialogs and widgets are cleaned up when closed.

## Extensibility Ideas
- Add custom parsing for CSV or JSON payloads by extending `onDataReceived()`.
- Introduce persistence for baud and range settings (e.g., `QSettings`).
- Support additional chart types (bars, scatter) via `QtCharts` components.
- Expand port filtering to cover non-macOS naming conventions or allow manual port entry.

## Deployment
- `CMakeLists.txt` uses `qt_generate_deploy_app_script` to produce a deployment script invoked during installation.
- Generated bundles can be packaged with Qt deployment tools (e.g., `macdeployqt`) for distribution.

## Testing Guidelines
- Use loopback or mock serial data providers to verify plotting without hardware.
- Exercise buffer mode, large values, and rapid data streams to confirm axis auto-scaling and trimming behavior.
- Validate configuration dialog interactions by toggling defaults and saving custom ranges.
