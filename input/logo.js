_ = {
  'gradients': {
    'white': '#990000',
    'black': '#000000',
    'white_text': '#ffffff',
    'hue': '#ffffff',
    'red': '#ffffff',
    'green': '#ffffff',
    'blue': '#9090ff',
  },
  'objects': {
    'logo_text': {
      'type': 'text',
      'gradient': 'white_text',
      'x': 0,
      'y': 0,
      'text': 'starcry',
      'text_size': 225,
      'text_align': 'center',
      'text_font': 'monogram.ttf',
      'text_fixed': false,
    },
    'logo_text_v2': {
      'type': 'text',
      'gradient': 'red',
      'x': 0,
      'y': 0,
      'text': 'starcry',
      'text_size': 220,
      'text_align': 'center',
      'text_font': 'monogram.ttf',
      'text_fixed': false,
    },
    'logo_art': {
      'type': 'circle',
      'gradient': 'blue',
      'x': 0,
      'y': 0,
      'scale': 0.8,
      'radius': 0,
      'radiussize': 20,
      'props': {'level': 0},
      'init': function() {
        if (this.props.level >= 2) return;
        let n = 8;
        for (let i = 0; i < n; i++) {
          let angle = (360. / n) * i;
          let newobj = this.spawn({
            'id': 'logo_art',
            'x': 8.5 * n,
            'y': 0,
            'z': 0,
            // 'radiussize': this.radiussize * 0.80,
            'opacity': 0.30,
            'angle': angle,
            'props': {'level': this.props.level + 1}
          });
          let line = this.spawn2(
              {
                'id': 'logo_line',
                'gradient': 'white_text',
                'opacity': 0.30,
              },
              newobj);
        }
      }
    },
    'logo_art_v2': {
      'type': 'circle',
      'gradient': 'blue',
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 21,
      'props': {'level': 0},
      'init': function() {
        if (this.props.level >= 2) return;
        let n = 6;
        for (let i = 0; i < n; i++) {
          let angle = (360. / n) * i;
          let newobj = this.spawn({
            'id': 'logo_art_v2',
            'x': 9 * n,
            'y': 0,
            'z': 0,
            'opacity': 1.0,
            'angle': angle,
            'props': {'level': this.props.level + 1}
          });
          let line =
              this.spawn2({'id': 'logo_line', 'gradient': 'white_text', 'radiussize': 2.5, 'opacity': 1.0}, newobj);
        }
        for (let i = 0; i < n; i++) {
          let angle = (360. / n) * i;
          for (let j = 0; j < 9; j++) {  // force real white
            let newobj2 = this.spawn({
              'id': 'logo_art_v2_sub',
              'x': 9 * n,
              'y': 0,
              'z': 0,
              'opacity': 1.0,
              'angle': angle,
              'props': {'level': this.props.level + 1}
            });
          }
        }
        // force real white in the center too
        this.spawn({
          'id': 'logo_art_v2_sub',
          'x': 0,
          'y': 0,
          'z': 0,
          'opacity': 1.0,
          'angle': 0,
        });
      },

    },
    'logo_art_v2_sub': {
      'type': 'circle',
      'gradient': 'blue',
      'blending_type': blending_type.normal,
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 17,
      'props': {'level': 0},
      'init': function() {}
    },
    'logo_art_bg': {
      'type': 'circle',
      'gradient': 'black',
      'x': 0,
      'y': 0,
      'radius': 96,
      'radiussize': 3,
      'props': {'level': 0},
    },
    'logo_art_bg_2': {
      'type': 'circle',
      'gradient': 'white_text',
      'x': 0,
      'y': 0,
      'radius': 128,
      'radiussize': 3,
      'props': {'level': 0},
      'init': function() {
        this.props.x = this.x;
      },
      'time': function(time) {
        var total_frames = 5 * 25.;
        var current_frame = total_frames * time;
        this.x = this.props.x + (this.props.direction * triangular_wave(current_frame, 1., 1.0) * 5);
      }
    },
    'logo_art_hue': {
      'type': 'circle',
      'gradient': 'hue',
      'blending_type': blending_type.color,
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 140,
      'props': {'level': 0},
      'init': function() {}
    },
    'logo_line': {
      'type': 'line',
      'blending_type': blending_type.normal,
      'gradient': 'white_text',
      'radius': 0,
      'radiussize': 10,
    },
  },
  'video': {
    'fps': 30,
    'width': 900,
    'height': 900,
    // TODO: why does this crash when rendering, some overflow w/r/t font bmp?
    // 'width': 900,
    // 'height': 300,
    'scale': 1.0,
    'rand_seed': 5,
    'granularity': 1,
    'min_intermediates': 1,
    'max_intermediates': 1,
    'grain_for_opacity': false,
    'bg_color': {'r': 0x99 / 255., 'g': 0, 'b': 0, 'a': 1},
    // 'bg_color': {'r': 0xb9 / 255., 'g': 0xf0 / 255., 'b': 0xde / 255., 'a': 1},
    // 'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 5,
      'objects': [
        {'id': 'logo_art_bg', 'x': -250, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'logo_art_bg_2', 'x': -250, 'y': 0, 'z': 0, 'props': {'direction': 1.}},
        {'id': 'logo_art_bg_2', 'x': -250, 'y': 0, 'z': 0, 'props': {'direction': -1.}},
        //{'id': 'logo_art', 'x': -200, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'logo_art_v2', 'x': 250, 'y': 0, 'z': 0, 'angle': 180, 'props': {}},
        //{'id': 'logo_text_v2', 'x': 160, 'y': 25, 'z': 0, 'props': {}},
        {'id': 'logo_text', 'x': 160, 'y': 25, 'z': 0, 'props': {}},
        // {'id': 'logo_art_hue', 'x': -250, 'y': 0, 'z': 0, 'props': {}},
      ]
    },
  ]
};
