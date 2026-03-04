#pragma once

#include <optional>
#include <limits>

#include "MetricHistory.h"

namespace eclipse::telemetry::history {

    //simple stats computed from a MetricHistory.
    struct HistoryStats {
        double min = 0.0;
        double max = 0.0;
        double avg = 0.0;
        std::size_t count = 0;
    };

    //compute min/max/avg for all points currently stored in history.
    //returns std::nullopt if there are no points.
    template <std::size_t Capacity>

    //inline to prevent errors for multilple #includes. either returns history stats or nullopt
    inline std::optional<HistoryStats> computeStats(const MetricHistory<Capacity>& h)
    {
        //if there is not history (no points stored, return nullopt
        if (h.empty())
            return std::nullopt;

        //create stats objec
        //initialises all members
        HistoryStats s{};
        //stores how many points exists
        s.count = h.size();

        //tracking vars. Any real number will be less than +infinity and greater than -inifinity
        double minV = std::numeric_limits<double>::infinity();
        double maxV = -std::numeric_limits<double>::infinity();
        double sum = 0.0;

        //loop over all points
        for (std::size_t i = 0; i < h.size(); ++i)
        {
            //get the value
            const double v = h.point(i).v;
            //update the min and max
            if (v < minV) minV = v;
            if (v > maxV) maxV = v;
            //update the sum
            sum += v;
        }
        //assign results
        s.min = minV;
        s.max = maxV;
        //compute the average
        s.avg = (s.count > 0) ? (sum / static_cast<double>(s.count)) : 0.0;
        //return the result.
        return s;
    }

} 