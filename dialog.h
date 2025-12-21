#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}
class SerialDevice;
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

signals:
    void portSelected(const QString &portName);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    SerialDevice *serialDevice;
};

#endif // DIALOG_H
