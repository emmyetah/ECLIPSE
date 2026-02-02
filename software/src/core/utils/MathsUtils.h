//clamp, lerp, moving average helpers
//safe min/max updated percent-of-ramge calc

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace core::math {

    // Clamp a value to a given range.
    // Example: Clamp(120, 0, 100) -> 100
    template<typename T>
    constexpr T Clamp(T value, T min, T max) {
        return value < min ? min : (value > max ? max : value);
    }

    // Linear interpolation between a and b by t in [0,1].
    // Example: Lerp(0.0, 100.0, 0.25) -> 25.0
    template<typename T>
    constexpr T Lerp(T a, T b, double t) {
        return a + static_cast<T>((b - a) * t);
    }

    // Safely compute value / max, returning 0 if max <= 0.
    // Useful for progress bars and percentages.
    template<typename T>
    constexpr double SafeRatio(T value, T max) {
        return (max > static_cast<T>(0))
            ? static_cast<double>(value) / static_cast<double>(max)
            : 0.0;
    }

    // Compute percentage of value within a range [min, max].
    // Result is clamped to [0,1].
    template<typename T>
    constexpr double PercentOfRange(T value, T min, T max) {
        if (max <= min) return 0.0;
        return Clamp(
            static_cast<double>(value - min) / static_cast<double>(max - min),
            0.0,
            1.0
        );
    }

    // Update a running minimum.
    template<typename T>
    inline void UpdateMin(T& currentMin, T value) {
        if (value < currentMin) currentMin = value;
    }

    // Update a running maximum.
    template<typename T>
    inline void UpdateMax(T& currentMax, T value) {
        if (value > currentMax) currentMax = value;
    }

    // Returns true if two floating-point values are approximately equal.
    inline bool NearlyEqual(double a, double b, double epsilon = 1e-6) {
        return std::fabs(a - b) <= epsilon;
    }

}
