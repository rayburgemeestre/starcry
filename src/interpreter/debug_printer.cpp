/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "interpreter/debug_printer.h"
#include "generator.h"

namespace interpreter {

debug_printer::debug_printer(scenes& scenes) : scenes_(scenes) {}

void debug_printer::debug_print_all() {
  logger(DEBUG) << "==[ debug print: next (" << scenes_.next_shapes_current_scene().size() << ") ]==" << std::endl;
  debug_print(scenes_.next_shapes_current_scene());

  logger(DEBUG) << "==[ debug print: intermediate (" << scenes_.intermediate_shapes_current_scene().size()
                << ") ]==" << std::endl;
  debug_print(scenes_.intermediate_shapes_current_scene());

  logger(DEBUG) << "==[ debug print: current (" << scenes_.shapes_current_scene().size() << ") ]==" << std::endl;
  debug_print(scenes_.shapes_current_scene());
}

void debug_printer::debug_print_next() {
  debug_print(scenes_.next_shapes_current_scene());
}

void debug_printer::debug_print(std::vector<data_staging::shape_t>& shapes) {
  const auto print_meta = [](const data_staging::meta& meta,
                             const data_staging::location& loc,
                             const data_staging::behavior& beh,
                             const data_staging::generic& gen,
                             const data_staging::styling& sty) {
    logger(INFO) << "uid=" << meta.unique_id() << ", puid=" << meta.parent_uid() << ", id=" << meta.id()
                 << ", level=" << meta.level() << ", namespace=" << meta.namespace_name() << " @ "
                 << loc.position_cref().x << "," << loc.position_cref().y << ", last_collide=" << beh.last_collide()
                 << ", mass=" << gen.mass() << ", angle = " << gen.angle() << ", gravity_group=" << beh.gravity_group()
                 << ", opacity=" << gen.opacity() << ", texture = " << sty.texture()
                 << ", gradient = " << sty.gradient() << ", unique_group = " << beh.unique_group()
                 << ", destroyed = " << std::boolalpha << meta.is_destroyed() << ", scale = " << gen.scale()
                 << std::endl;
  };
  for (auto& shape : shapes) {
    meta_callback(shape, [&]<typename T>(const T& cc) {
      print_meta(cc.meta_cref(), cc.location_cref(), cc.behavior_cref(), cc.generic_cref(), cc.styling_cref());
    });
  }
}

}  // namespace interpreter