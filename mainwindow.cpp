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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serialDevice(new SerialDevice(this))
{
    ui->setupUi(this);

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


    // Code for the graph

    series = new QLineSeries();
    QChart *chart = new QChart();
    axisX = new QValueAxis();
    axisY = new QValueAxis();
    chart->addSeries(series);
    axisX->setRange(0,10);
    axisY->setRange(0,10);
    chart->setTitle("Value Chart");
    axisX->setTitleText("Time (s)");
    axisY->setTitleText("Value");
    chart->addAxis(axisX,Qt::AlignBottom);
    chart->addAxis(axisY,Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->legend()->hide();
    chart->setMargins(QMargins(0, 0, 0, 0));

    chart->setBackgroundRoundness(0);
    chart->setBackgroundBrush(QBrush(QColor("#121212")));
    chart->setTitleBrush(QBrush(Qt::white));

    QChartView *chartView = new QChartView(chart);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    chartView->setRenderHint(QPainter::Antialiasing);

    ui->groupBox->layout()->addWidget(chartView);

    graphTimer = new QTimer(this);
    graphTimer->start(1000);


    connect(graphTimer, &QTimer::timeout, this, [this](){
        int valueToPlot = latestValue;

        if(bufferMode){
            if(dataBuffer.isEmpty()){
                return ;
            }
            valueToPlot = dataBuffer.last();
        }


        if(ui->pushButton_start->text()=="Turn Off"){
        timeCounter++;


        series->append(timeCounter,valueToPlot);

        const int maxPointsKept = 600;
        if (series->count() > maxPointsKept) {
            series->removePoints(0, series->count() - maxPointsKept);
        }

        //Scroling effect;
        axisX->setRange(qMax(0, timeCounter-10),timeCounter);

        if(valueToPlot>0){
            maxY = qMax(qMax(maxY,valueToPlot+5),10);
        }else{
            maxY = qMax(qMax(maxY,(-1*valueToPlot)+5),10);
        }

        axisY->setRange((-1*maxY),maxY);
        dataBuffer.clear();
        }



    });
}



MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_pushButton_start_clicked()
{
    QString text = ui->pushButton_start->text();

    if (text == "Turn On") {
        if (selectedPortName.isEmpty()) {
            QStringList ports = serialDevice->availablePorts();

            if (ports.isEmpty()) {
                ui->statusbar->showMessage("No serial ports found");
                return;
            }

            selectedPortName = ports.first();
            ui->label_port->setText("Port Selected : " + selectedPortName);
        }

        if (!serialDevice->connectToPort(selectedPortName, BaudValue)) {
            ui->statusbar->showMessage("Failed to connect");
            return;
        }

        ui->pushButton_start->setText("Turn Off");
    }
    else {
        serialDevice->disconnectPort();
        ui->pushButton_start->setText("Turn On");
    }
}

void MainWindow::onPortSelected(const QString &portName)
{
    selectedPortName = portName;
    ui->label_port->setText("Port Selected : " + portName);

    if (serialDevice->connectToPort(portName, BaudValue)) {
        ui->statusbar->showMessage("Connected to " + portName);
        ui->pushButton_start->setText("Turn Off");
    } else {
        ui->statusbar->showMessage("Failed to connect");
        ui->pushButton_start->setText("Turn On");
    }
}

void MainWindow::on_pushButton_clicked()
{
    Dialog *sec = new Dialog(this);
    connect(sec, &Dialog::portSelected,
            this, &MainWindow::onPortSelected);
    sec->exec();
}

void MainWindow::onDataReceived(const QByteArray &data)
{
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

    qDebug() << "Received:" << value;
    latestValue = value;
    dataBuffer.push_back(latestValue);
    ui->lcdNumber->display(latestValue);
}




void MainWindow::on_pushButton_2_clicked()
{
    const QString pressedStyle = "background-color: green;";

    if(ui->pushButton_2->text()=="Enable Buffer Mode"){
        bufferMode = true;
        ui->pushButton_2->setText("Disable Buffer Mode");
        ui->pushButton_2->setStyleSheet(pressedStyle);


    }
    else{
        bufferMode = false;
        ui->pushButton_2->setText("Enable Buffer Mode");
        ui->pushButton_2->setStyleSheet("");
    }

}



void MainWindow::on_actionSet_Config_triggered()
{
    Config *conf = new Config(this);
    conf->setInitialValues(BaudValue, maxY);
    connect(conf, &Config::baudSent,
            this, [this](int data) {
                BaudValue = data;
            });

    connect(conf, &Config::maxRSent,
            this, [this](int data) {
                maxY = data;
                axisY->setRange((-1*maxY),maxY);
            });
    conf->exec();
}

