#include "fonts/monaco.h"

ALLEGRO_FILE * initialize_monaco_font()
{
    // NOTE: Do not apply function caching here, ownership of the memfile is taken over by the ALLEGRO_FILE object.
    // Which will also assume it can free it, also returning the same memfile for each ALLEGRO_FILE causes segfaults.
    // Therefore apply caching only for the ALLEGRO_FILE's.
    return al_open_memfile(static_cast<void *>(Monaco_Linux_Powerline_ttf), Monaco_Linux_Powerline_ttf_len * sizeof(char), "r");
}
