#pragma once

#include <QWidget>
#include <QString>
#include <QVector>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class TrendPlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrendPlotWidget(QWidget* parent = nullptr);

    //setter funcitons for graph
    void SetPlotTitle(const QString& title);
    void SetYAxisTitle(const QString& title);
    void SetSeriesName(const QString& name);
    void SetData(const QVector<double>& values);
    void SetMetricData(const QString& plotTitle, const QString& yAxisTitle, const QVector<double>& values);
    void ClearData();

private:
    void BuildUi();
    void SetupChart();
    void SetupAxes();
    void ApplyChartStyle();
    void UpdateAxisRanges();

private:
    QtCharts::QChart* chart_;
    QtCharts::QChartView* chartView_;
    QtCharts::QLineSeries* series_;
    QtCharts::QValueAxis* axisX_;
    QtCharts::QValueAxis* axisY_;

    QVector<double> data_;
};