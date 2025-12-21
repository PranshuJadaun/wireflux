# WireFlux Serial Monitor

WireFlux is a Qt 6 desktop application for monitoring serial data (e.g., Arduino/ESP boards) and visualizing it in real time with a scrolling line chart.

## Features
- Serial port discovery and selection dialog
- Start/stop controls with status updates
- Optional buffer mode for smoothed plotting
- Live chart with auto-scaling axes and point trimming
- Configuration dialog to adjust baud rate and Y-axis range
- LCD readout of the latest received sample

## Use Cases
- Plot sensor streams from Arduino, ESP, or other microcontroller boards at common baud values such as 9,600, 57,600, or 115,200 bps.
- Monitor lab or hobby projects that emit newline-separated numeric data for quick visualization without additional tooling.
- Give students or makers an easy interface for experimenting with live serial plots during workshops or hackathons.
- Prepare for upcoming enhancements that will broaden protocol support and visualization options (more coming soon).

## Getting Started
### Prerequisites
- CMake 3.19+
- Qt 6.5+ with modules: Core, Widgets, SerialPort, Charts
- C++17-capable compiler supported by Qt

### Installation (Prebuilt Bundle)
- <a href="https://github.com/PranshuJadaun/wireflux/blob/main/Installables/wireflux.dmg?raw=1"><img src="https://img.shields.io/badge/Download-macOS%20DMG-blue?style=for-the-badge" alt="Download for macOS"/></a>
- *macOS-only build*
- Download the latest WireFlux app bundle/DMG from the releases page or provided installables.
- Open the disk image (if applicable) and drag the WireFlux application into your `Applications` folder.
- Launch WireFlux from `Applications`; macOS Gatekeeper may request confirmation on first run.

### Build
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="/path/to/Qt/6.5.0/macos"
cmake --build build
```

### Run
- Launch the generated application (bundle on macOS).
- Connect a serial device streaming newline-separated integers.
- Use **Select Port** to choose the device, then **Turn On** to start plotting.

## How to Use WireFlux
- **Connect hardware**: Attach your Arduino/ESP or compatible serial device and ensure it is outputting newline-separated numeric values.
- **Start the app**: Launch WireFlux (installed bundle or build output).
- **Select port**: Click **Select Port**, choose the desired entry from the list, and confirm. The status bar will display the connection state.
- **Adjust settings (optional)**: Open **Menu → Set Config** to change the baud rate or Y-axis range; save or revert to defaults as needed.
- **Begin plotting**: Click **Turn On** to start streaming. The LCD shows the latest value while the chart scrolls with live data.
- **Buffer mode**: Use **Enable Buffer Mode** to smooth bursts by plotting buffered values on each timer tick; disable to return to immediate plotting.
- **Stop or disconnect**: Click **Turn Off** to end the session or unplug the device; the status bar updates when disconnected.

## Documentation
See `DOCUMENTATION.md` for in-depth module descriptions, architecture diagrams, and workflow details.

## License
