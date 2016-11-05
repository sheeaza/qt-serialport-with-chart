#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QWidget>
#include <QScrollBar>

namespace Ui {
class MainWindow;
}

class ChartWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void showPortInfo(int idx);

    void on_connectionButton_clicked();
    void on_baudrateBox_currentIndexChanged(int index);
    void on_databitsBox_currentIndexChanged(int index);
    void on_parityBox_currentIndexChanged(int index);
    void on_stopbitsBox_currentIndexChanged(int index);
    void on_serialPortBox_currentIndexChanged(int index);

    void serialPortErrorHandle(QSerialPort::SerialPortError error);

    //receive data
    void serialDataToRcvTextChar();
    void serialDataToRcvTextHex();
    void rcvTextFormSwitch(int state);

    //send data
    void sendDataToserialChar();
    void sendDataToserialHex();
    void sendTextFormSwitch(int state);

signals:
    void sendToChart(QByteArray &byteArray);

protected:
    bool nativeEvent(const QByteArray &, void *message, long *result);

private:
    void openSerialport();
    void closeSerialport();
    void fillPortsParameters();
    void fillPortsInfo();
    void convertQSerialInfoToQSlist(const QSerialPortInfo &info,
                                    QStringList &list);
    void initConnections();
    void clearPortInfo();
    void smartInserText(const QString &);

    Ui::MainWindow *ui;
    QSerialPort *serial;
    ChartWindow *chart;
};

#endif // MAINWINDOW_H
