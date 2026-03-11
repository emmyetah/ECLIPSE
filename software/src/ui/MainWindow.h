#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QChart>
#include <QLegend>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;
    //for trend plot graph
    QChart* trendChart_ = nullptr;
    QChartView* trendChartView_ = nullptr;
    QLineSeries* trendSeries_ = nullptr;
    QValueAxis* trendAxisX_ = nullptr;
    QValueAxis* trendAxisY_ = nullptr;

    void SetupTrendChart();
    void LoadTestTrendData();
};
#endif // MAINWINDOW_H
