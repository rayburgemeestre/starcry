/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstddef>
#include <vector>
#include <memory>

namespace FastPForLib {
    class IntegerCODEC;
}

template <typename T>
class compress_vector
{
private:
    std::shared_ptr<FastPForLib::IntegerCODEC> codec_ptr;

public:
    compress_vector();
    void compress(std::vector<T> *input_ptr, double *compression_rate = nullptr);
    void decompress(std::vector<T> *input_ptr, size_t target_vec_len);
};
