#ifndef CHARTLINE_H
#define CHARTLINE_H

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QDialog>
#include <QtWidgets>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class ChartWindow;
}

QT_CHARTS_USE_NAMESPACE

class ChartWindow : public QDialog
{
    Q_OBJECT
public:
    ChartWindow(QWidget *parent = 0);
    ~ChartWindow();

public slots:
    void writeData(QByteArray &byteArray);

private slots:
    void updateYmax();
    void updateYmin();
    void updateXmax();
    void updateXmin();

private:
    QLineSeries *m_series;
    QChart *m_chart;
    QChartView *m_chartView;
    QValueAxis *axisX;
    QValueAxis *axisY;
    Ui::ChartWindow *ui;
};

#endif // CHARTLINE_H
