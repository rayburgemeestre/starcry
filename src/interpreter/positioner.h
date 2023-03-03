/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace interpreter {
    class generator;

    class positioner {
    private:
        generator& gen_;

    public:
        positioner(generator& gen);
        void update_object_positions();
    };

}  // namespace interpreter
