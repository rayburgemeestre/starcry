#pragma once

#include "shape.hpp"

namespace data {

struct job {
    size_t job_number;
    size_t frame_number;
    bool rendered;
    bool last_frame;
    size_t chunk;
    size_t num_chunks;
    std::vector<shape<double>> shapes;
};

}
