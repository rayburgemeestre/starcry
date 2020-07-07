/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <memory>
#include <mutex>

static std::mutex mut__;

class a {
public:
  explicit a(std::ostream &os) : _holder(std::make_shared<holder>(os)) {}

  template <class T>
  friend std::ostream &operator<<(const a &l, const T &t) {
    return (l._holder->_os) << t;
  }

private:
  struct holder {
    explicit holder(std::ostream &os) : _os(os), _lock(mut__) {}

    std::ostream &_os;
    std::lock_guard<std::mutex> _lock;
  };

  mutable std::shared_ptr<holder> _holder;
};
