/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

// needed since we don't have std::clamp in <algorithm> with em++ currently
template <class T>
inline constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
  //  assert(!(hi < lo));
  return (v < lo) ? lo : (hi < v) ? hi : v;
}

static constexpr const auto pi = 3.14159265358979323846;
