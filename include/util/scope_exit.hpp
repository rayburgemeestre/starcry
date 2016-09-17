/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

template <typename T>
class scope_exit
{
private:
    T func_;
public:

    scope_exit(T f) : func_(f)
    {
    }

    ~scope_exit() {
        func_();
    }
};
