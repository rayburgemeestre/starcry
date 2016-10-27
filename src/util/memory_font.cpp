/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "util/memory_font.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_memfile.h>
#include "fonts/monaco.h"

#include <iostream>

memory_font::memory_font(fonts font, int size, int flags)
  : font_(font), size_(size), flags_(flags), allegro_file_(nullptr, nullptr), allegro_font_(nullptr, nullptr)
{
    switch (font_) {
        case fonts::monaco:
            initialize(Monaco_Linux_Powerline_ttf, Monaco_Linux_Powerline_ttf_len, "Monaco_Linux-Powerline.ttf");
            break;
        default:
            std::cerr << "memory_font invalid font" << std::endl;
            std::exit(1);
    }
}

void memory_font::initialize(unsigned char * font_data, size_t font_data_len, const char * font_filename)
{
    // NOTE: Do not apply function caching here, ownership of the memfile is taken over by the ALLEGRO_FILE object.
    // Which will also assume it can free it, also returning the same memfile for each ALLEGRO_FILE causes segfaults.
    // Therefore apply caching only for the ALLEGRO_FILE's.
    allegro_file_ = std::unique_ptr<ALLEGRO_FILE, decltype(&al_fclose)>(al_open_memfile(static_cast<void *>(font_data),
                                                                  font_data_len * sizeof(char),
                                                                  "r"), [](ALLEGRO_FILE *) -> bool {
                                                                            // al_fclose() is already called by the font
                                                                            //  taking the ownership..
                                                                            return true;
                                                                        });

    allegro_font_ = std::unique_ptr<ALLEGRO_FONT, decltype(&al_destroy_font)>(al_load_ttf_font_f(allegro_file_.get(),
                                                                                                 font_filename,
                                                                                                 size_,
                                                                                                 flags_),
                                                                              &al_destroy_font);

}
