#include "KpiCardWidget.h"

#include <algorithm>

#include <QLabel>
#include <QSlider>
#include <QFrame>

#include "../../viewmodel/KpiCardVm.h"
#include "../../logic/thresholds/ThresholdLevel.h"

namespace eclipse::ui::widgets {

    KpiCardWidget::KpiCardWidget()
    {
    }

    void KpiCardWidget::Bind(
        QLabel* metricLabel,
        QLabel* valueLabel,
        QLabel* minLabel,
        QLabel* maxLabel,
        QLabel* statusLabel,
        QLabel* timestampLabel,
        QSlider* indicatorSlider,
        QFrame* topStatusBar
    ){
        metricLabel_ = metricLabel;
        valueLabel_ = valueLabel;
        minLabel_ = minLabel;
        maxLabel_ = maxLabel;
        statusLabel_ = statusLabel;
        timestampLabel_ = timestampLabel;
        indicatorSlider_ = indicatorSlider;
        topStatusBar_ = topStatusBar;

        if (indicatorSlider_ != nullptr) {
            indicatorSlider_->setMinimum(0);
            indicatorSlider_->setMaximum(100);
            indicatorSlider_->setValue(0);
            indicatorSlider_->setEnabled(false);
        }
    }

    bool KpiCardWidget::IsBound() const
    {
        return metricLabel_ != nullptr
            && valueLabel_ != nullptr
            && minLabel_ != nullptr
            && maxLabel_ != nullptr
            && statusLabel_ != nullptr
            && timestampLabel_ != nullptr
            && indicatorSlider_ != nullptr;
    }

    void KpiCardWidget::Update(const eclipse::viewmodel::KpiCardVm& vm)
    {
        if (!IsBound()) {
            return;
        }

        metricLabel_->setText(QString::fromStdString(vm.GetLabel()));
        valueLabel_->setText(QString::fromStdString(vm.GetValueText()));

        minLabel_->setText(
            //g for general format, 4 for precision (number of significant digits)
            "min:" + QString::number(vm.GetDisplayMin(), 'f', 1)
        );

        maxLabel_->setText(
            "max:" + QString::number(vm.GetDisplayMax(), 'f', 1)
        );

        eclipse::logic::thresholds::ThresholdLevel level = vm.GetLevel();
        statusLabel_->setText(LevelText(level));
        timestampLabel_->setText(QString::fromStdString(vm.GetTimestampText()));

        double normalized = vm.GetNormalizedValue();
        normalized = std::clamp(normalized, 0.0, 1.0);

        int sliderValue = static_cast<int>(normalized * 100.0);
        indicatorSlider_->setValue(sliderValue);

        ApplyLevelStyle(level);
    }

    void KpiCardWidget::SetTimestampText(const QString& text)
    {
        if (timestampLabel_ == nullptr) {
            return;
        }

        timestampLabel_->setText(text);
    }

    void KpiCardWidget::SetRangeText(const QString& minText, const QString& maxText)
    {
        if (minLabel_ != nullptr) {
            minLabel_->setText(minText);
        }

        if (maxLabel_ != nullptr) {
            maxLabel_->setText(maxText);
        }
    }

    QString KpiCardWidget::LevelText(eclipse::logic::thresholds::ThresholdLevel level) const
    {
        using eclipse::logic::thresholds::ThresholdLevel;

        if (level == ThresholdLevel::Normal) {
            return "NORMAL";
        }
        if (level == ThresholdLevel::Caution) {
            return "CAUTION";
        }
        if (level == ThresholdLevel::Warning) {
            return "WARNING";
        }
        if (level == ThresholdLevel::Critical) {
            return "CRITICAL";
        }

        return "UNKNOWN";
    }

    void KpiCardWidget::ApplyLevelStyle(eclipse::logic::thresholds::ThresholdLevel level)
    {
        if (statusLabel_ == nullptr || indicatorSlider_ == nullptr || topStatusBar_ == nullptr) {
            return;
        }

        QString color = "#8A9199";

        using eclipse::logic::thresholds::ThresholdLevel;

        if (level == ThresholdLevel::Normal) {
            color = "#57C785";
        }
        else if (level == ThresholdLevel::Caution) {
            color = "#F0C14B";
        }
        else if (level == ThresholdLevel::Warning) {
            color = "#FF9F43";
        }
        else if (level == ThresholdLevel::Critical) {
            color = "#FF5C5C";
        }

        statusLabel_->setStyleSheet(
            "color: " + color + ";"
            "font-size: 12px;"
            "font-weight: 600;"
            "background: transparent;"
            "border: none;"
        );

        indicatorSlider_->setStyleSheet(
            "QSlider::groove:horizontal {"
            "    background: #252B31;"
            "    height: 6px;"
            "    border-radius: 3px;"
            "}"
            "QSlider::handle:horizontal {"
            "    background: " + color + ";"
            "    width: 14px;"
            "    height: 14px;"
            "    margin: -4px 0;"
            "    border-radius: 7px;"
            "    border: 2px solid #111315;"
            "}"
            "QSlider::sub-page:horizontal {"
            "    background: #252B31;"
            "    border-radius: 3px;"
            "}"
            "QSlider::add-page:horizontal {"
            "    background: #252B31;"
            "    border-radius: 3px;"
            "}"
        );

        topStatusBar_->setStyleSheet(
            "background-color: " + color + ";"
        );
    }

}