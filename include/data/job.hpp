#pragma once

#include "shape.hpp"

namespace data {

struct job {
    uint32_t width;
    uint32_t height;
    uint32_t offset_x;
    uint32_t offset_y;
    size_t job_number;
    size_t frame_number;
    bool rendered;
    bool last_frame;
    size_t chunk;
    size_t num_chunks;
    std::vector<shape<double>> shapes;
};

}
