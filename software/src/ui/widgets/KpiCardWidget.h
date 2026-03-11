#pragma once

#include <QString>

class QLabel;
class QSlider;
class QFrame;

namespace eclipse::logic::thresholds {
    enum class ThresholdLevel : unsigned char;
}

namespace eclipse::viewmodel {
    class KpiCardVm;
}

namespace eclipse::ui::widgets {

    //wraps one KPI card that already exists in MainWindow.ui
    class KpiCardWidget {
    public:
        //creates empty widget wrapper
        KpiCardWidget();

        //connectes wrapper to the ui
        void Bind(
            QLabel* metricLabel,
            QLabel* valueLabel,
            QLabel* minLabel,
            QLabel* maxLabel,
            QLabel* statusLabel,
            QLabel* timestampLabel,
            QSlider* indicatorSlider,
            QFrame* topStatusBar

        );

        //checks that all required UI widgets were successfully connected
        bool IsBound() const;

        //takes kpi card view model and updates the entire kpi card using the data
        void Update(const eclipse::viewmodel::KpiCardVm& vm);

        //updates timestamp label on kpi card, comes form telemetry snapshot.
        void SetTimestampText(const QString& text);
        //sets the min & max labels
        void SetRangeText(const QString& minText, const QString& maxText);

    private:
        //converts threshold level enum inot human readable string
        QString LevelText(eclipse::logic::thresholds::ThresholdLevel level) const;
        //updates vidual styling of kpi card based on severity
        void ApplyLevelStyle(eclipse::logic::thresholds::ThresholdLevel level);

    private:
        QLabel* metricLabel_ = nullptr;
        QLabel* valueLabel_ = nullptr;
        QLabel* minLabel_ = nullptr;
        QLabel* maxLabel_ = nullptr;
        QLabel* statusLabel_ = nullptr;
        QLabel* timestampLabel_ = nullptr;
        QSlider* indicatorSlider_ = nullptr;
        QFrame* topStatusBar_ = nullptr;
    };

}