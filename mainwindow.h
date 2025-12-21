#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QObject>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QTimer>
#include <QVector>
#include <QString>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class SerialDevice;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();


private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_clicked();

    void onPortSelected(const QString &portName);
    void onDataReceived(const QByteArray &data);


    void on_pushButton_2_clicked();

    void on_actionSet_Config_triggered();

private:
    Ui::MainWindow *ui;
    SerialDevice *serialDevice;
    int latestValue=0;
    QLineSeries *series;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QTimer *graphTimer;
    int timeCounter=0;
    int maxY=10;
    QVector<int> dataBuffer;
    bool bufferMode = false;
    int BaudValue=9600;
    QString selectedPortName;

};
#endif // MAINWINDOW_H
