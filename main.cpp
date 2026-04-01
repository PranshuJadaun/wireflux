/**
 * @file main.cpp
 * @brief Application entry point for WireFlux — a Qt 6 serial monitor and ECG analysis tool.
 *
 * This file bootstraps the QApplication, applies a global dark theme stylesheet
 * using a raw string literal, and instantiates the MainWindow.
 *
 * The dark theme uses a curated color palette:
 * - Base background:   #1A1A24 (deep navy-black)
 * - Panel background:  #232330 (slightly lighter)
 * - Input background:  #12121A (darkest)
 * - Primary accent:    #00A8E8 (vibrant cyan)
 * - Text color:        #E0E0E0 (soft white)
 * - Border color:      #3A3A4A (subtle gray)
 *
 * @see MainWindow
 */

#include "mainwindow.h"

#include <QApplication>
#include <QSerialPort>
#include <QSerialPortInfo>

/**
 * @brief Program entry point.
 *
 * Initializes the Qt event loop, applies the premium dark theme to all widgets
 * via QApplication::setStyleSheet(), creates and shows the MainWindow, then
 * enters the blocking event loop.
 *
 * @param argc Command-line argument count.
 * @param argv Command-line argument values.
 * @return Exit code from QApplication::exec().
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // ──────────────────────────────────────────────────────────────────
    // Global Dark Theme Stylesheet
    // Applied to the entire application via QApplication::setStyleSheet().
    // Individual widgets inherit these styles unless explicitly overridden.
    // ──────────────────────────────────────────────────────────────────
    QString darkTheme = R"(
        QWidget {
            background-color: #1A1A24;
            color: #E0E0E0;
            font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif;
            font-size: 13px;
        }
        QMainWindow::separator {
            background: #2D2D3B;
            width: 1px;
            height: 1px;
        }
        QGroupBox {
            border: 1px solid #3A3A4A;
            border-radius: 6px;
            margin-top: 1ex;
            background-color: #232330;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
            color: #A0A0B0;
        }
        QPushButton {
            background-color: #2D2D3B;
            border: 1px solid #3A3A4A;
            border-radius: 4px;
            padding: 6px 16px;
            color: #FFFFFF;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #3A3A4A;
            border: 1px solid #4A4A5A;
        }
        QPushButton:pressed {
            background-color: #00A8E8;
            border: 1px solid #0086B8;
            color: #FFFFFF;
        }
        QLineEdit, QComboBox {
            background-color: #12121A;
            border: 1px solid #3A3A4A;
            border-radius: 4px;
            padding: 4px 8px;
            color: #FFFFFF;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #00A8E8;
        }
        QComboBox::drop-down {
            border-left: 1px solid #3A3A4A;
        }
        QStatusBar {
            background-color: #12121A;
            color: #A0A0B0;
        }
        QMenuBar {
            background-color: #12121A;
            color: #E0E0E0;
        }
        QMenuBar::item:selected {
            background-color: #2D2D3B;
        }
        QMenu {
            background-color: #232330;
            border: 1px solid #3A3A4A;
        }
        QMenu::item:selected {
            background-color: #00A8E8;
        }
        QLCDNumber {
            background-color: #12121A;
            border: 1px solid #3A3A4A;
            border-radius: 6px;
            color: #00A8E8;
        }
        QLabel {
            background-color: transparent;
        }
    )";
    
    a.setStyleSheet(darkTheme);

    MainWindow w;
    w.show();
    return a.exec();
}
