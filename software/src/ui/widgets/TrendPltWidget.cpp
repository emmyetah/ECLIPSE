#include "TrendPltWidget.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QPen>
#include <QtMath>
#include <algorithm>

TrendPlotWidget::TrendPlotWidget(QWidget* parent)
    : QWidget(parent)
{
    chart_ = nullptr;
    chartView_ = nullptr;
    series_ = nullptr;
    axisX_ = nullptr;
    axisY_ = nullptr;

    BuildUi();
    SetupChart();
    SetupAxes();
    ApplyChartStyle();
}

void TrendPlotWidget::BuildUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    chartView_ = new QChartView(this);
    chartView_->setRenderHint(QPainter::Antialiasing, true);
    chartView_->setFrameShape(QFrame::NoFrame);
    chartView_->setStyleSheet("background-color: rgb(28, 31, 36); border: none;");
    setMinimumSize(900, 230);
    setMaximumHeight(250);

    mainLayout->addWidget(chartView_);
    setLayout(mainLayout);
}

void TrendPlotWidget::SetupChart()
{
    series_ = new QLineSeries();
    chart_ = new QChart();

    chart_->addSeries(series_);
    chart_->legend()->hide();
    chart_->setTitle("Trend Plot");

    chartView_->setChart(chart_);
}

void TrendPlotWidget::SetupAxes()
{
    axisX_ = new QValueAxis();
    axisY_ = new QValueAxis();

    axisX_->setTitleText("Samples");
    axisX_->setLabelFormat("%d");
    axisX_->setTickCount(6);
    axisX_->setRange(0, 10);

    axisY_->setTitleText("Temperature(°C)");
    axisY_->setLabelFormat("%.1f");
    axisY_->setTickCount(6);
    axisY_->setRange(0.0, 100.0);

    chart_->addAxis(axisX_, Qt::AlignBottom);
    chart_->addAxis(axisY_, Qt::AlignLeft);

    series_->attachAxis(axisX_);
    series_->attachAxis(axisY_);
}

void TrendPlotWidget::ApplyChartStyle()
{
    chart_->setBackgroundVisible(true);
    chart_->setBackgroundRoundness(12);
    chart_->setPlotAreaBackgroundVisible(false);
    chart_->setMargins(QMargins(4, 4, 4, 4));

    QBrush chartBackground(QColor("#1C1F24"));
    chart_->setBackgroundBrush(chartBackground);

    QFont titleFont;
    titleFont.setPointSize(11);
    titleFont.setBold(false);
    chart_->setTitleFont(titleFont);
    chart_->setTitleBrush(QBrush(QColor("#EAF4FF")));

    QPen seriesPen(QColor("#58D68D"));
    seriesPen.setWidth(3);
    series_->setPen(seriesPen);

    axisX_->setLabelsColor(QColor("#B7C4D6"));
    axisY_->setLabelsColor(QColor("#B7C4D6"));

    axisX_->setTitleBrush(QBrush(QColor("#EAF4FF")));
    axisY_->setTitleBrush(QBrush(QColor("#EAF4FF")));

    QFont axisTitleFont;
    axisTitleFont.setPointSize(9);
    axisTitleFont.setBold(true);
    axisX_->setTitleFont(axisTitleFont);
    axisY_->setTitleFont(axisTitleFont);

    QPen axisLinePen(QColor("#445066"));
    axisLinePen.setWidth(1);
    axisX_->setLinePen(axisLinePen);
    axisY_->setLinePen(axisLinePen);

    QPen gridPen(QColor("#2B3444"));
    gridPen.setWidth(1);
    axisX_->setGridLinePen(gridPen);
    axisY_->setGridLinePen(gridPen);

    QPen minorGridPen(QColor("#202836"));
    minorGridPen.setWidth(1);
    axisX_->setMinorGridLinePen(minorGridPen);
    axisY_->setMinorGridLinePen(minorGridPen);

    axisX_->setGridLineVisible(true);
    axisY_->setGridLineVisible(true);
    axisX_->setMinorGridLineVisible(false);
    axisY_->setMinorGridLineVisible(false);

    chartView_->setBackgroundBrush(QBrush(Qt::transparent));
}

void TrendPlotWidget::SetPlotTitle(const QString& title)
{
    chart_->setTitle(title);
}

void TrendPlotWidget::SetYAxisTitle(const QString& title)
{
    axisY_->setTitleText(title);
}

void TrendPlotWidget::SetSeriesName(const QString& name)
{
    series_->setName(name);
}

void TrendPlotWidget::SetData(const QVector<double>& values)
{
    data_ = values;
    series_->clear();

    for (int i = 0; i < data_.size(); i++)
    {
        series_->append(i, data_[i]);
    }

    UpdateAxisRanges();
}

void TrendPlotWidget::SetMetricData(const QString& plotTitle, const QString& yAxisTitle, const QVector<double>& values)
{
    SetPlotTitle(plotTitle);
    SetYAxisTitle(yAxisTitle);
    SetData(values);
}

void TrendPlotWidget::ClearData()
{
    data_.clear();
    series_->clear();

    axisX_->setRange(0, 10);
    axisY_->setRange(0.0, 100.0);
}

void TrendPlotWidget::UpdateAxisRanges()
{
    if (data_.isEmpty())
    {
        axisX_->setRange(0, 10);
        axisY_->setRange(0.0, 100.0);
        return;
    }

    double minValue = data_[0];
    double maxValue = data_[0];

    for (int i = 1; i < data_.size(); i++)
    {
        if (data_[i] < minValue)
        {
            minValue = data_[i];
        }

        if (data_[i] > maxValue)
        {
            maxValue = data_[i];
        }
    }

    double padding = (maxValue - minValue) * 0.15;

    //checks if floating point numbers are almost equal within a tiny tolerance (qt helper funciton)
    if (qFuzzyCompare(minValue, maxValue))
    {
        padding = std::max(1.0, std::abs(minValue) * 0.1);
    }

    axisX_->setRange(0, static_cast<int>(std::max<qsizetype>(1, data_.size() - 1)));
    axisY_->setRange(minValue - padding, maxValue + padding);
}