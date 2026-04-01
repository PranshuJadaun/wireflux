# WireFlux

> **Real-time serial data monitor & ECG analysis tool** built with Qt 6

WireFlux is a desktop application for monitoring serial data streams from microcontroller boards (Arduino, ESP32, etc.) and visualizing them as a live scrolling chart. It includes a specialized **ECG Mode** for cardiac signal analysis with real-time BPM calculation and cloud-based ML classification.

---

## 🚀 What's New in v2.0

> Upgrading from the [v1.0 release on GitHub](https://github.com/PranshuJadaun/wireflux). Below is a complete list of everything that changed.

### ✨ New Features (not in v1.0)

| Feature | Details |
|---|---|
| **ECG Mode** | Dedicated cardiac monitoring mode with 10 ms graph refresh (vs 1000 ms), 8 ms per-sample timing for 125 Hz data, and paper-speed proportional display (~150 px/sec) |
| **R-Peak Detection** | Real-time threshold-based peak detector with 20-unit hysteresis to identify heartbeats from raw ECG signals |
| **BPM Calculation** | Rolling 5-beat average heart rate derived from RR intervals, displayed on a dedicated LCD. Valid range: 30–200 BPM |
| **RR Interval Display** | Millisecond-precision RR interval between consecutive R-peaks shown on a separate LCD |
| **ECG Recording** | One-click recording captures exactly 200 samples into an ML buffer for cloud analysis |
| **ML-Powered Classification** | Automatic POST of 200-sample ECG buffers to `https://ecgbackend.onrender.com/predict` — returns Normal/Abnormal with confidence score |
| **ECG Results Panel** | New UI group (`ecgGroup`) with BPM LCD, RR LCD, threshold display, recording status, API result label (color-coded), and confidence label |
| **Configurable ECG Threshold** | Peak detection threshold is now adjustable at runtime via the Config dialog (default: 580) |
| **Central Value Setting** | New "Central Value" config parameter shifts the Y-axis center reference — essential for ECG signals that don't oscillate around zero |
| **Central Value Guard** | Safety check prevents `centralR >= maxY` (would invert the chart), shows error dialog and rolls back |
| **Premium Dark Theme** | Global QSS stylesheet applied in `main.cpp` with curated color palette (#1A1A24 base, #00A8E8 accent), covering all widget types |

### 🔧 Changed / Improved (from v1.0)

| Component | v1.0 (GitHub) | v2.0 (Current) |
|---|---|---|
| **Qt Modules** | Core, Widgets, SerialPort, Charts | + **Network** (for HTTP API calls) |
| **Graph Timer** | Fixed 1000 ms, integer `timeCounter++` | Dual-mode: 1000 ms normal / 10 ms ECG; `double currentTime` for sub-second precision |
| **Buffer Mode** | Plots only `dataBuffer.last()` | Plots **ALL** buffered values with 8 ms spacing — no data loss at high frequency |
| **Point Trim Limit** | 600 points max | **5,000 points** max (supports 4K displays and longer ECG traces) |
| **X-Axis Scroll** | Integer-based `qMax(0, timeCounter-10)` | Double-based `qMax(0.0, currentTime - windowXSpan)` with dynamic span |
| **Y-Axis Scaling** | Symmetric only: `[-maxY, +maxY]` | **Asymmetric ECG mode**: `[maxY - 2*(maxY-centralR), maxY]` centered on `centralR` |
| **Button Names** | Generic (`pushButton`, `pushButton_start`, `pushButton_2`) | Semantic names (`btnSelectPort`, `btnConnectionToggle`, `btnBufferToggle`) |
| **Slot Names** | `on_pushButton_start_clicked()`, etc. | `on_btnConnectionToggle_clicked()`, etc. |
| **Buffer Toggle Style** | `background-color: green` | Accent style: `#00A8E8` background with matching border |
| **Config Dialog** | 2 parameters (baud, maxY) via `setInitialValues(int, int)` | **4 parameters** (baud, maxY, centralR, threshold) via `setInitialValues(int, int, int, int)` |
| **Config Signals** | `baudSent`, `maxRSent` | + `centralSent`, `thresholdSent` |
| **Config Defaults** | `DefaultBaudRate`, `DefaultMaxRange` | + `DefaultCentral = 0`, `DefaultThreshold = 580` |
| **Dialog Memory** | Heap-allocated (`new Dialog(this)`) — potential leak | **Stack-allocated** (`Dialog sec(this)`) — automatic cleanup |
| **Config Memory** | Heap-allocated (`new Config(this)`) — potential leak | **Stack-allocated** (`Config conf(this)`) — automatic cleanup |
| **Debug Logging** | `qDebug()` on every received value (UI lag at 125 Hz) | Aggressive logging **removed** from hot path; API payloads still logged |



### 📦 New Dependencies

| Dependency | Purpose |
|---|---|
| `Qt6::Network` | `QNetworkAccessManager`, `QNetworkRequest`, `QNetworkReply` for ECG API communication |

### 🖥️ New UI Widgets (in `mainwindow.ui`)

| Widget | Type | Purpose |
|---|---|---|
| `ecgModeButton` | `QPushButton` | Toggles ECG mode on/off |
| `ecgGroup` | `QGroupBox` | Collapsible panel for all ECG-related controls |
| `lcdBPM` | `QLCDNumber` | Displays rolling average BPM |
| `lcdRR` | `QLCDNumber` | Displays RR interval in ms |
| `labelBPM` | `QLabel` | "BPM :" label |
| `labelRR` | `QLabel` | "RR Interval :" label |
| `labelThreshold` | `QLabel` | Shows current threshold value |
| `ecgRec` | `QPushButton` | Toggles ECG recording |
| `labelRecStatus` | `QLabel` | Recording progress / status |
| `ecgResult` | `QLabel` | API classification result (color-coded) |
| `ecgConf` | `QLabel` | API confidence score |
| `SampleData` | `QLabel` | Displays last 10 recorded samples |

---

## Features

### Serial Monitoring
- **Auto-discovery** of available serial ports via system enumeration
- **Port selection dialog** with combo-box for easy switching
- **Configurable baud rates**: 9600 · 14400 · 19200 · 28800 · 38400 · 57600 · 115200
- **Newline-delimited protocol** — expects integer values terminated by `\n`

### Real-Time Graphing
- **Live scrolling chart** powered by QtCharts (`QLineSeries` + `QChartView`)
- **Auto-scaling Y-axis** that expands dynamically to accommodate data spikes
- **Sliding X-axis window** displaying the most recent time span
- **Anti-aliased rendering** for smooth visuals on all displays
- **Point trimming** — limits to 5,000 data points to prevent memory bloat

### Buffer Mode
- **Batch plotting** — buffers all incoming samples and plots them together on each timer tick
- Ideal for high-frequency data streams where individual 1-second ticks would drop data
- Toggle on/off with a styled button (accent highlight when active)

### ECG Mode
- **125 Hz optimized rendering** — graph updates every 10 ms, each sample mapped to 8 ms
- **Paper-speed proportional display** — 1 second of data = ~150 pixels, regardless of window width
- **R-Peak detection** with configurable threshold and 20-unit hysteresis
- **BPM calculation** — rolling 5-beat average (valid range: 30–200 BPM)
- **RR interval display** in milliseconds

### ML-Powered ECG Classification
- **Record 200 samples** with one-click ECG recording
- **Cloud inference** via the [ECG Backend API](https://github.com/PranshuJadaun/ecgBackend) hosted on Render
- **Result display** — "Normal" (green) / "Abnormal" (red) with confidence score
- **Error handling** for network failures and malformed API responses

#### API Endpoints

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `https://ecgbackend.onrender.com/` | Health check — returns `{"status": "ECG API Running"}` |
| `POST` | `https://ecgbackend.onrender.com/predict` | ECG classification — accepts 200 samples, returns Normal/Abnormal with confidence |

> **Backend repo**: [PranshuJadaun/ecgBackend](https://github.com/PranshuJadaun/ecgBackend) — FastAPI + TensorFlow/Keras, deployed on Render

### Configuration
- **Runtime-adjustable** baud rate, Y-axis max, central value, and ECG threshold
- **Revert to defaults** button for quick reset
- **Input validation** with `QIntValidator` for numeric fields

## Screenshots

*Connect your device and launch WireFlux to see the dark-themed interface with live chart, LCD readout, and ECG panel.*

## Getting Started

### Prerequisites

| Requirement | Version |
|---|---|
| CMake | 3.19+ |
| Qt | 6.5+ |
| Qt Modules | Core, Widgets, SerialPort, Charts, Network |
| Compiler | C++17 capable |

### Install (Prebuilt — macOS)

<a href="https://github.com/PranshuJadaun/wireflux/blob/main/Installables/wireflux.dmg?raw=1"><img src="https://img.shields.io/badge/Download-macOS%20DMG-blue?style=for-the-badge" alt="Download for macOS"/></a>

1. Download the DMG from the link above or from `Installables/`
2. Open the disk image and drag WireFlux to your `Applications` folder
3. Launch WireFlux — macOS Gatekeeper may prompt on first run

### Build from Source

```bash
# Configure (adjust the Qt path for your installation)
cmake -S . -B build -DCMAKE_PREFIX_PATH="/path/to/Qt/6.5.0/macos"

# Build
cmake --build build

# Run
open build/untitled.app      # macOS
./build/untitled              # Linux
```

## Usage Guide

### Basic Serial Monitoring

1. **Connect hardware** — Attach your Arduino/ESP or compatible serial device
2. **Select port** — Click `Select Port`, choose the correct entry, and confirm
3. **Start streaming** — Click `Turn On` to begin receiving data
4. **View data** — The LCD displays the latest sample; the chart scrolls live
5. **Adjust settings** — Use `Menu → Set Config` to change baud rate or axis range
6. **Buffer mode** — Toggle `Enable Buffer Mode` for high-frequency data smoothing
7. **Stop** — Click `Turn Off` or disconnect the device

### ECG Analysis

1. **Enable ECG Mode** — Click `ECG Mode : Off` to switch to `ECG Mode : On`
2. **Connect sensor** — Ensure your ECG sensor (e.g., AD8232) is streaming at ~125 Hz
3. **Monitor vitals** — Observe real-time BPM and RR interval on the LCD panels
4. **Adjust threshold** — Use `Menu → Set Config` to tune the peak detection threshold
5. **Record for ML** — Click `ECG Recording : Off` to start capturing 200 samples
6. **Get prediction** — Once 200 samples are captured, the app automatically sends data to the cloud API and displays the Normal/Abnormal result with confidence

## Project Structure

```
wireflux/
├── main.cpp              # App entry point, global dark theme
├── mainwindow.h/cpp/ui   # Core controller: serial, chart, ECG, API
├── serialdevice.h/cpp    # QSerialPort abstraction layer
├── dialog.h/cpp/ui       # Port selection dialog
├── config.h/cpp/ui       # Configuration dialog

├── resources.qrc         # Qt resource file (embeds E.png)
├── E.png                 # Application logo
├── CMakeLists.txt        # Build configuration
├── DOCUMENTATION.md      # Full technical documentation
├── Installables/         # Prebuilt macOS DMG
└── .gitignore            # Build artifacts, IDE metadata
```

## Documentation

See [`DOCUMENTATION.md`](DOCUMENTATION.md) for comprehensive technical documentation including:
- Full architecture diagrams (Mermaid)
- Complete signal-slot mapping
- Data pipeline walkthrough
- Peak detection algorithm details
- API request/response format
- UI layout reference with widget names

## Tech Stack

- **Language**: C++17
- **Framework**: Qt 6.5+
- **Modules**: Core · Widgets · SerialPort · Charts · Network
- **Build**: CMake 3.19+
- **Backend**: [ecgBackend](https://github.com/PranshuJadaun/ecgBackend) — FastAPI + TensorFlow/Keras (hosted on [Render](https://ecgbackend.onrender.com/))

## ECG Backend

The ML classification feature relies on a separate backend service:

| | |
|---|---|
| **Repository** | [github.com/PranshuJadaun/ecgBackend](https://github.com/PranshuJadaun/ecgBackend) |
| **Framework** | FastAPI (Python) |
| **ML Model** | `ecg_sliding_model.keras` — Keras/TensorFlow CNN |
| **Hosted at** | [ecgbackend.onrender.com](https://ecgbackend.onrender.com/) |
| **Input** | JSON `{"data": [200 float values]}` |
| **Output** | JSON `{"result": "Normal"/"Abnormal", "confidence": 0.0–1.0}` |
| **Threshold** | Prediction > 0.4 → Abnormal, else Normal |
| **Preprocessing** | Z-score normalization (`(x - mean) / std`) |

## License

*No license specified*
