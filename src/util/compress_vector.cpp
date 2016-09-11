/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "util/compress_vector.h"
#include "headers/codecfactory.h"
#include "headers/deltautil.h"

using namespace FastPForLib;

template <typename T>
compress_vector<T>::compress_vector()
    : codec_ptr(CODECFactory::getFromName("simdfastpfor256"))
{
}

template <typename T>
void compress_vector<T>::compress(std::vector<T> *input_ptr, double *compression_rate) {
        vector<T> &input = *input_ptr;
        vector<uint32_t> compressed_output(input.size() + (1024* 100));
        size_t compressedsize = compressed_output.size();
        codec_ptr->encodeArray(input.data(), input.size(), compressed_output.data(), compressedsize);
        compressed_output.resize(compressedsize);
        compressed_output.shrink_to_fit();
        if (compression_rate != nullptr) {
            *compression_rate = 100.0 * static_cast<double>(compressed_output.size()) / static_cast<double>(input.size());
        }
        input = std::move(compressed_output);
}

template <typename T>
void compress_vector<T>::decompress(std::vector<T> *input_ptr, size_t target_vec_len) {
        vector<T> &input = *input_ptr;
        std::vector<uint32_t> mydataback(target_vec_len);
        size_t recoveredsize = mydataback.size();
        codec_ptr->decodeArray(input.data(), input.size(), mydataback.data(), recoveredsize);
        mydataback.resize(recoveredsize);
        input = std::move(mydataback);

}

template class compress_vector<uint32_t>;


