#include "MainWindow.h"
#include "./ui_mainwindow.h"

#include <QVBoxLayout>

#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SetupTrendChart();
    LoadTestTrendData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupTrendChart()
{
    trendSeries_ = new QLineSeries(this);

    trendChart_ = new QChart();
    trendChart_->addSeries(trendSeries_);
    trendChart_->legend()->hide();
    trendChart_->setTitle("Temperature Trend");
    trendChart_->setBackgroundVisible(true);
    trendChart_->setPlotAreaBackgroundVisible(false);

    trendAxisX_ = new QValueAxis();
    trendAxisX_->setTitleText("Samples");
    trendAxisX_->setRange(0, 10);
    trendAxisX_->setLabelFormat("%d");
    trendAxisX_->setGridLineVisible(true);

    trendAxisY_ = new QValueAxis();
    trendAxisY_->setTitleText("Value");
    trendAxisY_->setRange(0, 50);
    trendAxisY_->setLabelFormat("%.1f");
    trendAxisY_->setGridLineVisible(true);

    trendChart_->addAxis(trendAxisX_, Qt::AlignBottom);
    trendChart_->addAxis(trendAxisY_, Qt::AlignLeft);

    trendSeries_->attachAxis(trendAxisX_);
    trendSeries_->attachAxis(trendAxisY_);

    trendChartView_ = new QChartView(trendChart_, this);
    trendChartView_->setRenderHint(QPainter::Antialiasing);

    // Put chart view inside your UI container
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->trendChartContainer->layout());
    if (layout == nullptr) {
        layout = new QVBoxLayout(ui->trendChartContainer);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }

    layout->addWidget(trendChartView_);
}

void MainWindow::LoadTestTrendData()
{
    trendSeries_->clear();

    trendSeries_->append(0, 22.1);
    trendSeries_->append(1, 22.4);
    trendSeries_->append(2, 22.8);
    trendSeries_->append(3, 22.5);
    trendSeries_->append(4, 23.0);
    trendSeries_->append(5, 23.4);
    trendSeries_->append(6, 23.1);
    trendSeries_->append(7, 22.9);
    trendSeries_->append(8, 23.3);
    trendSeries_->append(9, 23.7);

    trendAxisX_->setRange(0, 9);
    trendAxisY_->setRange(20, 25);
}

