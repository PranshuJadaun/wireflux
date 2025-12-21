#include "config.h"
#include "ui_config.h"
#include <QPixmap>
#include <QIntValidator>


Config::Config(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Config)
{
    ui->setupUi(this);

    //QPixmap pic("qrc:/images/E.png");
    //int h = ui->label_pic->height();
    //int w = ui->label_pic->width();
    //ui->label_pic->setPixmap(pic.scaled(100,100,Qt::KeepAspectRatio));
    ui->comboBox_BaudList->addItem("9600");
    ui->comboBox_BaudList->addItem("14400");
    ui->comboBox_BaudList->addItem("19200");
    ui->comboBox_BaudList->addItem("28800");
    ui->comboBox_BaudList->addItem("38400");
    ui->comboBox_BaudList->addItem("57600");
    ui->comboBox_BaudList->addItem("115200");

    ui->lineEdit_Max->setValidator(new QIntValidator(1, 1000000, this));
    ui->lineEdit_Max->setText(QString::number(maxR));

    const int defaultIndex = ui->comboBox_BaudList->findText(QString::number(baudRate));
    if (defaultIndex >= 0) {
        ui->comboBox_BaudList->setCurrentIndex(defaultIndex);
    }
}

Config::~Config()
{
    delete ui;
}

void Config::on_pushButton_saveClose_clicked()
{
    baudRate = ui->comboBox_BaudList->currentText().toInt();
    maxR = ui->lineEdit_Max->text().toInt();

    emit baudSent(baudRate);
    emit maxRSent(maxR);

    accept();  // close dialog
}


void Config::on_pushButton_setDefault_clicked()
{
    baudRate = DefaultBaudRate;
    maxR = DefaultMaxRange;

    const int defaultIndex = ui->comboBox_BaudList->findText(QString::number(DefaultBaudRate));
    if (defaultIndex >= 0) {
        ui->comboBox_BaudList->setCurrentIndex(defaultIndex);
    }

    ui->lineEdit_Max->setText(QString::number(DefaultMaxRange));
}


void Config::setInitialValues(int baud, int maxRange)
{
    if (baud > 0) {
        baudRate = baud;
    }
    if (maxRange > 0) {
        maxR = maxRange;
    }

    const int index = ui->comboBox_BaudList->findText(QString::number(baudRate));
    if (index >= 0) {
        ui->comboBox_BaudList->setCurrentIndex(index);
    }

    ui->lineEdit_Max->setText(QString::number(maxR));
}

