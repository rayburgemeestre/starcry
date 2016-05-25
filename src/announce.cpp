/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "common.h"

#include "announce.h"

#include "data/job.hpp"
#include "data/shape.hpp"
#include "data/pixels.hpp"

using namespace data;

namespace data {
    void announce() {
        caf::announce<job>(
            "job",
            &job::width,
            &job::height,
            &job::canvas_w,
            &job::canvas_h,
            &job::offset_x,
            &job::offset_y,
            &job::job_number,
            &job::frame_number,
            &job::rendered,
            &job::last_frame,
            &job::chunk,
            &job::num_chunks,
            &job::shapes,
            &job::scale
        );
        caf::announce<gradient>(
            "gradient",
            &gradient::colors
        );
        caf::announce<shape>(
            "shape",
            &shape::x,
            &shape::y,
            &shape::z,
            &shape::x2,
            &shape::y2,
            &shape::z2,
            &shape::type,
            &shape::r,
            &shape::g,
            &shape::b,
            &shape::radius,
            &shape::radius_size,
            &shape::text,
            &shape::align,
            &shape::gradient_
        );
        caf::announce<ALLEGRO_COLOR>(
            "ALLEGRO_COLOR",
            &ALLEGRO_COLOR::r,
            &ALLEGRO_COLOR::g,
            &ALLEGRO_COLOR::b,
            &ALLEGRO_COLOR::a
        );
        caf::announce<pixel_data>(
            "pixel_data",
            &pixel_data::pixels
        );
    }

}

