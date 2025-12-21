#ifndef CONFIG_H
#define CONFIG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class Config;
}

class Config : public QDialog
{
    Q_OBJECT

public:
    explicit Config(QWidget *parent = nullptr);
    ~Config();

    void setInitialValues(int baud, int maxRange);

private slots:
    void on_pushButton_saveClose_clicked();
    void on_pushButton_setDefault_clicked();

signals:
    void baudSent(int baud);
    void maxRSent(int maxR);

private:
    static constexpr int DefaultBaudRate = 9600;
    static constexpr int DefaultMaxRange = 10;
    Ui::Config *ui;
    //QPixmap *pic = new QPixmap();
    int baudRate = DefaultBaudRate;
    int maxR = DefaultMaxRange;

};

#endif // CONFIG_H
