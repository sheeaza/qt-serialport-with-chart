#include "chartwindow.h"
#include "ui_chartwindow.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

ChartWindow::ChartWindow(QWidget *parent):
    QDialog(parent),
    m_series(0),
    ui(new Ui::ChartWindow)
{
    ui->setupUi(this);
    m_chart = new QChart;
    m_chartView = new QChartView(m_chart);
    m_chartView->setMinimumSize(800, 600);
    m_series = new QLineSeries;
    m_chart->addSeries(m_series);

    QPen green(Qt::red);
    green.setWidth(1);
    m_series->setPen(green);

    axisX = new QValueAxis;
    axisX->setRange(0,100);
    axisX->setLabelFormat("%g");
    axisX->setTitleText(QStringLiteral("Samples"));
    axisY = new QValueAxis;
    axisY->setRange(0,100);
    axisY->setTitleText("Serial Data");
    ui->yMaxLineEdit->setText(QString::number(axisY->max()));
    ui->yMinLineEdit->setText(QString::number(axisY->min()));
    ui->xMaxLineEdit->setText(QString::number(axisX->max()));
    ui->xMinLineEdit->setText(QString::number(axisX->min()));

    m_chart->setAxisX(axisX, m_series);
    m_chart->setAxisY(axisY, m_series);
    m_chart->legend()->hide();
    m_chart->setTitle(QStringLiteral("Data from Serial"));

    ui->chartLayout->addWidget(m_chartView);

    connect(ui->yMaxLineEdit, &QLineEdit::editingFinished,
            this, &ChartWindow::updateYmax);
    connect(ui->yMinLineEdit, &QLineEdit::editingFinished,
            this, &ChartWindow::updateYmin);
    connect(ui->xMaxLineEdit, &QLineEdit::editingFinished,
            this, &ChartWindow::updateXmax);
    connect(ui->xMinLineEdit, &QLineEdit::editingFinished,
            this, &ChartWindow::updateXmin);
}

ChartWindow::~ChartWindow()
{
    delete ui;
}

void ChartWindow::writeData(QByteArray &byteArray)
{
    if (!ui->pauseButton->isChecked()) {
    qint64 range = static_cast<qint64>(axisX->max() - axisX->min());
    QVector<QPointF> oldPoints = m_series->pointsVector();
    QVector<QPointF> points;

    if (oldPoints.count() < range) {
        points = m_series->pointsVector();
    } else {
        for (int i = byteArray.size();
             i < oldPoints.count(); i++)
            points.append(QPointF(i - byteArray.size(),
                                 oldPoints.at(i).y()));
    }

    qint64 size = points.count();
    for (int k = 0; k < byteArray.size(); k++)
        points.append(QPointF(k + size, (quint8)byteArray.at(k)));
    m_series->replace(points);
    }
}

void ChartWindow::updateYmax()
{
    bool ok;
    qreal value;

    value = ui->yMaxLineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisY())->setMax(value);
    } else {
        ui->yMaxLineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        ui->yMaxLineEdit->setText(QString::number(axisY->max()));
        ui->yMaxLineEdit->blockSignals(false);
    }
}

void ChartWindow::updateYmin()
{
    bool ok;
    qreal value;

    value = ui->yMinLineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisY())->setMin(value);
    } else {
        ui->yMinLineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        ui->yMinLineEdit->setText(QString::number(axisY->min()));
        ui->yMinLineEdit->blockSignals(false);
    }
}

void ChartWindow::updateXmax()
{
    bool ok;
    qreal value;

    value = ui->xMaxLineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisX())->setMax(value);
    } else {
        ui->xMaxLineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        ui->xMaxLineEdit->setText(QString::number(axisX->max()));
        ui->xMaxLineEdit->blockSignals(false);
    }
}

void ChartWindow::updateXmin()
{
    bool ok;
    qreal value;

    value = ui->xMinLineEdit->text().toDouble(&ok);
    if (ok) {
        static_cast<QValueAxis*>(m_chart->axisX())->setMin(value);
    } else {
        ui->xMinLineEdit->blockSignals(true);
        QMessageBox::critical(this, tr("Critical Error"), tr("Invalid Value!"));
        ui->xMinLineEdit->setText(QString::number(axisX->min()));
        ui->xMinLineEdit->blockSignals(false);
    }
}
