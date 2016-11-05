#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#include <Windows.h>
#include <Dbt.h>
#include "chartwindow.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");
static const qint64 buffersize(2048);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serial = new QSerialPort(this);
    chart = new ChartWindow(this);

    //serial port configure
    connect(serial, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::serialPortErrorHandle);
    fillPortsParameters();
    fillPortsInfo();
    initConnections();

    //receive data text editor configure
    serial->setReadBufferSize(buffersize);
}

MainWindow::~MainWindow()
{
    serial->close();
    delete ui;
}

void MainWindow::fillPortsParameters()
{
    //baudrate
    ui->baudrateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudrateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudrateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudrateBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->baudrateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);

    //databits
    ui->databitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->databitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->databitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->databitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->databitsBox->setCurrentIndex(3);

    //parity
    ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    //stopbits
    ui->stopbitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopbitsBox->addItem(QStringLiteral("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopbitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
}

void MainWindow::fillPortsInfo()
{
    ui->serialPortBox->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        convertQSerialInfoToQSlist(info, list);

        ui->serialPortBox->addItem(list.first(), list);
    }
}

void MainWindow::showPortInfo(int idx)
{
    if (idx == -1)
        return;

    QStringList list = ui->serialPortBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ?
                                   list.at(1) : tr(blankString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ?
                                   list.at(2) : tr(blankString)));
    ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ?
                                   list.at(3) : tr(blankString)));
    ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ?
                                   list.at(4) : tr(blankString)));
    ui->viLabel->setText(tr("Vendor ID: %1").arg(list.count() > 5 ?
                                   list.at(5) : tr(blankString)));
    ui->piLabel->setText(tr("Product ID: %1").arg(list.count() > 6 ?
                                   list.at(6) : tr(blankString)));
}

void MainWindow::clearPortInfo()
{
    ui->descriptionLabel->setText(tr("Description: %1").arg(tr(blankString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(tr(blankString)));
    ui->serialNumberLabel->setText(tr("Serial number: %1").arg(tr(blankString)));
    ui->locationLabel->setText(tr("Location: %1").arg(tr(blankString)));
    ui->viLabel->setText(tr("Vendor ID: %1").arg(tr(blankString)));
    ui->piLabel->setText(tr("Product ID: %1").arg(tr(blankString)));
}

void MainWindow::initConnections()
{
    //receive connection init
    QObject::connect(ui->clearReceiveButton, &QPushButton::clicked,
                     ui->receiveTextEdit, &QPlainTextEdit::clear);

    QObject::connect(ui->hexCheckBoxR, &QCheckBox::stateChanged,
                     this, &MainWindow::rcvTextFormSwitch);
    if (ui->hexCheckBoxR->isChecked()) {
        QObject::connect(serial, &QSerialPort::readyRead,
                         this, &MainWindow::serialDataToRcvTextHex);
    } else {
        QObject::connect(serial, &QSerialPort::readyRead,
                         this, &MainWindow::serialDataToRcvTextChar);
    }

    //send connection init
    QObject::connect(ui->clearSendButton, &QPushButton::clicked,
                     ui->sendTextEdit, &QPlainTextEdit::clear);
    QObject::connect(ui->hexCheckBoxS, &QCheckBox::stateChanged,
                     this, &MainWindow::sendTextFormSwitch);
    if (ui->hexCheckBoxS->isChecked()) {
        QObject::connect(ui->sendDataButton, &QPushButton::clicked,
                         this, &MainWindow::sendDataToserialHex);
    } else {
        QObject::connect(ui->sendDataButton, &QPushButton::clicked,
                         this, &MainWindow::sendDataToserialChar);
    }

    //chart connection init
    connect(this, &MainWindow::sendToChart,
            chart, &ChartWindow::writeData, Qt::UniqueConnection);

    connect(ui->showChartButton, &QPushButton::clicked,
            chart, &ChartWindow::show);
}

void MainWindow::on_connectionButton_clicked()
{
    if (serial->isOpen()) {
        closeSerialport();
    } else {
        openSerialport();
    }
}

void MainWindow::on_baudrateBox_currentIndexChanged(int index)
{
    serial->setBaudRate(static_cast<QSerialPort::BaudRate>
                        (ui->baudrateBox->itemData(index).toInt()));
}

void MainWindow::on_databitsBox_currentIndexChanged(int index)
{
    serial->setDataBits(static_cast<QSerialPort::DataBits>
                        (ui->databitsBox->itemData(index).toInt()));
}

void MainWindow::on_parityBox_currentIndexChanged(int index)
{
    serial->setParity(static_cast<QSerialPort::Parity>
                        (ui->parityBox->itemData(index).toInt()));
}

void MainWindow::on_stopbitsBox_currentIndexChanged(int index)
{
    serial->setStopBits(static_cast<QSerialPort::StopBits>
                        (ui->stopbitsBox->itemData(index).toInt()));
}

void MainWindow::on_serialPortBox_currentIndexChanged(int index)
{
    MainWindow::showPortInfo(index);
}

void MainWindow::serialPortErrorHandle(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        MainWindow::closeSerialport();
    }
}

//return false if sucess other true
void MainWindow::openSerialport()
{
    bool ret;

    serial->setPortName(ui->serialPortBox->currentText());
    serial->setBaudRate(static_cast<QSerialPort::BaudRate>
                        (ui->baudrateBox->currentData().toInt()));
    serial->setDataBits(static_cast<QSerialPort::DataBits>
                        (ui->databitsBox->currentData().toInt()));
    serial->setParity(static_cast<QSerialPort::Parity>
                      (ui->parityBox->currentData().toInt()));
    serial->setStopBits(static_cast<QSerialPort::StopBits>
                        (ui->stopbitsBox->currentData().toInt()));
    ret = serial->open(QIODevice::ReadWrite);
    if (ret) {
        ui->serialPortBox->setDisabled(true);
        ui->connectionButton->setText(QStringLiteral("Disconnect"));
        ui->sendDataButton->setEnabled(true);
        ui->connectionButton->setCheckable(true);
        ui->connectionButton->setChecked(true);
    } else {
        ui->connectionButton->setCheckable(false);
    }
}

void MainWindow::closeSerialport()
{
    serial->close();
    ui->sendDataButton->setEnabled(false);
    ui->connectionButton->setChecked(false);
    ui->serialPortBox->setEnabled(true);
    ui->connectionButton->setText(QStringLiteral("Connect"));
}

bool MainWindow::nativeEvent(const QByteArray &, void *message, long *)
{
    MSG *msg = reinterpret_cast<MSG*>(message);
    PDEV_BROADCAST_HDR phdr = reinterpret_cast<PDEV_BROADCAST_HDR>(msg->lParam);

    if (msg->message == WM_DEVICECHANGE) {
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            if (phdr->dbch_devicetype == DBT_DEVTYP_PORT) {
                PDEV_BROADCAST_PORT pPort = reinterpret_cast<PDEV_BROADCAST_PORT>
                                            (phdr);
                const QSerialPortInfo serialPortInfo(QString::fromWCharArray
                                                     (pPort->dbcp_name));
                QStringList list;

                convertQSerialInfoToQSlist(serialPortInfo, list);
                ui->serialPortBox->addItem(list.first(), list);

//                qDebug() << "port arrival: "
//                         << QString::fromWCharArray(pPort->dbcp_name);
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if (phdr->dbch_devicetype == DBT_DEVTYP_PORT) {
                PDEV_BROADCAST_PORT pPort = reinterpret_cast<PDEV_BROADCAST_PORT>
                                            (phdr);
                int index = ui->serialPortBox->findText(QString::fromWCharArray
                                                        (pPort->dbcp_name));
                ui->serialPortBox->removeItem(index);
                if (ui->serialPortBox->count() == 0)
                    clearPortInfo();
            }
            break;
        }
    }
    return false;
}

void MainWindow::convertQSerialInfoToQSlist(const QSerialPortInfo &info, QStringList &list)
{
    QString description;
    QString manufacturer;
    QString serialNumber;

    description = info.description();
    manufacturer = info.manufacturer();
    serialNumber = info.serialNumber();
    list << info.portName()
         << (!description.isEmpty() ? description : blankString)
         << (!manufacturer.isEmpty() ? manufacturer : blankString)
         << (!serialNumber.isEmpty() ? serialNumber : blankString)
         << info.systemLocation()
         << (info.vendorIdentifier() ?
                 QString::number(info.vendorIdentifier(), 16) : blankString)
         << (info.productIdentifier() ?
                 QString::number(info.productIdentifier(), 16) : blankString);
}

void MainWindow::serialDataToRcvTextChar()
{
    QString string(serial->readAll());

    smartInserText(string);
}

void MainWindow::serialDataToRcvTextHex()
{
    QByteArray data(serial->readAll());
    QString string;

    for(int i = 0; i < data.size(); ++i) {
        string.append(QByteArray(1, data.at(i)).toHex().toUpper());
        string.append(' ');
    }
    smartInserText(string);

    emit sendToChart(data);
}

void MainWindow::smartInserText(const QString &text)
{
    QTextCursor prevCursor(ui->receiveTextEdit->textCursor());

    bool scrollbar;
    QScrollBar *bar = ui->receiveTextEdit->verticalScrollBar();
    int barValue = bar->value();
    if (bar->value() == bar->maximum()) {
        scrollbar = true;
    } else {
        scrollbar = false;
    }

    int size(ui->receiveTextEdit->toPlainText().size() + text.size());
    if (size > 1000) {
        QTextCursor cursor(ui->receiveTextEdit->textCursor());

        cursor.movePosition(QTextCursor::Start);
        cursor.setPosition(size - 1000, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }
    ui->receiveTextEdit->moveCursor(QTextCursor::End);
    ui->receiveTextEdit->insertPlainText(text);
    ui->receiveTextEdit->setTextCursor(prevCursor);

    if (scrollbar) {
        bar->setValue(bar->maximum());
    } else {
        bar->setValue(barValue);
    }
}

void MainWindow::rcvTextFormSwitch(int state)
{
    if (state == Qt::Unchecked) {
        QObject::disconnect(serial, &QSerialPort::readyRead,
                            this, &MainWindow::serialDataToRcvTextHex);
        QObject::connect(serial, &QSerialPort::readyRead,
                         this, &MainWindow::serialDataToRcvTextChar);

    } else if (state == Qt::Checked) {
        QObject::disconnect(serial, &QSerialPort::readyRead,
                            this, &MainWindow::serialDataToRcvTextChar);
        QObject::connect(serial, &QSerialPort::readyRead,
                         this, &MainWindow::serialDataToRcvTextHex);
    }
}

void MainWindow::sendDataToserialChar()
{
    QString string(ui->sendTextEdit->toPlainText());

    serial->write(&string.toStdString()[0], string.size());
}

void MainWindow::sendDataToserialHex()
{
    QString string(ui->sendTextEdit->toPlainText());

    QRegularExpression re("[^0-9a-fA-F\\s,]");
    QRegularExpressionMatch match = re.match(string);
    if (match.hasMatch()) {
         QMessageBox::critical(this, tr("Format Error"),
                               QStringLiteral("invalid value!"));
         return;
    }

    QStringList parts;
    QRegularExpressionMatchIterator i = QRegularExpression("[^\\s,]{1,2}").globalMatch(string);
    while (i.hasNext()) {
       parts << i.next().captured(0);
    }
    QByteArray hexArray;
    for (QString str: parts) {
        uchar hexChar;
        hexChar = str.toInt(nullptr, 16);
        hexArray.append(hexChar);
    }
    serial->write(hexArray);
}

void MainWindow::sendTextFormSwitch(int state)
{
    if (state == Qt::Unchecked) {
        QObject::disconnect(ui->sendDataButton, &QPushButton::clicked,
                            this, &MainWindow::sendDataToserialHex);
        QObject::connect(ui->sendDataButton, &QPushButton::clicked,
                         this, &MainWindow::sendDataToserialChar);
    } else if (state == Qt::Checked) {
        QObject::disconnect(ui->sendDataButton, &QPushButton::clicked,
                         this, &MainWindow::sendDataToserialChar);
        QObject::connect(ui->sendDataButton, &QPushButton::clicked,
                            this, &MainWindow::sendDataToserialHex);
    }
}
