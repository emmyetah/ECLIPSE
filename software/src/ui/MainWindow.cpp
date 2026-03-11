#include "MainWindow.h"
#include "./ui_mainwindow.h"
#include "widgets/TrendPltWidget.h"

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SetupTrendPlot();
    LoadTestTrendData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupTrendPlot()
{
    trendPlot_ = new TrendPlotWidget(this);

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->trendChartContainer->layout());
    if (layout == nullptr) {
        layout = new QVBoxLayout(ui->trendChartContainer);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
    }

    layout->addWidget(trendPlot_);
}

void MainWindow::LoadTestTrendData()
{
    QVector<double> testValues;
    testValues.append(22.1);
    testValues.append(22.4);
    testValues.append(22.8);
    testValues.append(22.5);
    testValues.append(23.0);
    testValues.append(23.4);
    testValues.append(23.1);
    testValues.append(22.9);
    testValues.append(23.3);
    testValues.append(23.7);

    trendPlot_->SetMetricData("Temperature Trend", "Value", testValues);
}