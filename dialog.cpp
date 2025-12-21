#include "dialog.h"
#include "ui_dialog.h"
#include "serialdevice.h"
#include <QDebug>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , serialDevice(new SerialDevice(this))
{
    ui->setupUi(this);
    QStringList ports = serialDevice->availablePorts();
    ui->label_ports->setText(ports.join("\n"));

    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (info.portName().startsWith("cu.usb")) {
            qDebug() << "ESP/Arduino Port:" << info.portName();
            ui->comboBox_selectPort->addItem(info.portName());
        }
    }

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    QString selectedPort = ui->comboBox_selectPort->currentText();
    emit portSelected(selectedPort);
    accept();
}

