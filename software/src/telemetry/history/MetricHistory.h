#pragma once

#include <cstddef>
#include <optional>

#include "../../core/types/Time.h"
#include "../MetricId.h"
#include "RingBuffer.h"

namespace eclipse::telemetry::history {


    //A single point for charting: (time, value)
    struct HistoryPoint {
        core::time::TimePoint t{}; //steady time 
        double v = 0.0;            //metric value
    };

    //for storing a rolling window of points for plotting
    template <std::size_t Capacity>
    class MetricHistory {
    public:
        MetricHistory() = default;

        //clears all history points
        void clear() {
            _points.clear();
        }

        //adds a new point to history
        void add(const core::time::TimePoint& t, double v) {
            _points.push(HistoryPoint{ t, v });
        }

        //how many points are currently stored
        std::size_t size() const {
            return _points.size();
        }

        //returns true if no points stored
        bool empty() const {
            return _points.empty();
        }

        //returns the i-th point (0 = oldest, size-1 = newest)
        const HistoryPoint& point(std::size_t i) const {
            return _points.at(i);
        }

        //returns newest point if available
        std::optional<HistoryPoint> latest() const {
            if (_points.empty()) return std::nullopt;
            return _points.at(_points.size() - 1);
        }

        //returns oldest point if available
        std::optional<HistoryPoint> oldest() const {
            if (_points.empty()) return std::nullopt;
            return _points.at(0);
        }

    private:
        RingBuffer<HistoryPoint, Capacity> _points{}; // rolling storage
    };

} 