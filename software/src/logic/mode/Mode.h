#pragma once

namespace eclipse::logic::mode {

    //defines the operating mode of the system. controls which logic rules and threshold sets are used
    enum class Mode {
        Earth, // real sensor more with indorr thresholds
        Space  //smulated mode with harsher environmental conditions
    };

}