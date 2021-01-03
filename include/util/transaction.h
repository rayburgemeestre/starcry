/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

class transaction {
public:
  transaction() = default;
  virtual ~transaction() = default;

  virtual void reset() = 0;
  // virtual void update() = 0;
  virtual void revert() = 0;
  virtual void commit() = 0;
};
