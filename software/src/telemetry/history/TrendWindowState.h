#pragma once

#include <chrono>

namespace eclipse::telemetry::history
{

    enum class TrendWindow
    {
        OneMinute,
        FiveMinutes,
        FifteenMinutes,
        OneHour,
        AllTime
    };

    inline std::chrono::seconds WindowDuration(TrendWindow window)
    {
        switch (window)
        {
        case TrendWindow::OneMinute:
            return std::chrono::minutes(1);

        case TrendWindow::FiveMinutes:
            return std::chrono::minutes(5);

        case TrendWindow::FifteenMinutes:
            return std::chrono::minutes(15);

        case TrendWindow::OneHour:
            return std::chrono::hours(1);

        case TrendWindow::AllTime:
            return std::chrono::hours(24);

        default:
            return std::chrono::minutes(5);
        }
    }

}