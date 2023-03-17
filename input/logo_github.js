_ = {
  'gradients': {
    'primary': '#ff0000',
    'black': '#000000',
    'white2': '#ffffff',
  },
  'objects': {
    'logo_text': {
      'type': 'text',
      'gradient': 'black',
      'text': 'starcry',
      'text_size': 225,
      'text_align': 'center',
      'text_font': 'monogram.ttf',
      'text_fixed': false,
    },
    'circles_pattern': {
      'type': 'circle',
      'gradient': 'primary',
      'radius': 0,
      'radiussize': 21,
      'props': {'level': 0},
      'init': function() {
        if (this.props.level >= 2) return;
        let n = 6;
        for (let i = 0; i < n; i++) {
          let angle = (360. / n) * i;
          let newobj = this.spawn({
            'id': 'circles_pattern',
            'x': 9 * n,
            'y': 0,
            'z': 0,
            'opacity': 1.0,
            'angle': angle,
            'props': {'level': this.props.level + 1}
          });
          this.spawn2({'id': 'logo_line', 'gradient': 'primary', 'radiussize': 2.5, 'opacity': 1.0}, newobj);
        }
        for (let i = 0; i < n; i++) {
          let angle = (360. / n) * i;
          // force real white
          for (let j = 0; j < 9; j++) {
            this.spawn({
              'id': 'white_circle',
              'x': 9 * n,
              'y': 0,
              'z': 0,
              'opacity': 1.0,
              'angle': angle,
              'props': {'level': this.props.level + 1}
            });
          }
        }
        // force real white in center
        for (let j = 0; j < 3; j++)
          this.spawn({
            'id': 'white_circle',
            'x': 0,
            'y': 0,
            'z': 0,
            'opacity': 1.0,
            'angle': 0,
          });
      },
    },
    'white_circle': {'type': 'circle', 'gradient': 'white2', 'radius': 0, 'radiussize': 17, 'init': function() {}},
    'black_circle': {
      'type': 'circle',
      'gradient': 'black',
      'radius': 96,
      'radiussize': 3,
    },
    'outer_circle': {
      'type': 'circle',
      'gradient': 'primary',
      'x': 0,
      'y': 0,
      'radius': 128,
      'radiussize': 3,
      'init': function() {
        this.props.x = this.x;
      },
      'time': function(time) {
        let total_frames = 5 * 25.;
        let current_frame = total_frames * time;
        this.x = this.props.x + (this.props.direction * triangular_wave(current_frame, 1., 1.0) * 5);
      }
    },
    'logo_line': {
      'type': 'line',
      'gradient': 'primary',
      'radius': 0,
      'radiussize': 10,
    },
  },
  'video': {
    'fps': 30,
    'width': 800,
    'height': 280,
    'scale': 3.9,
    'rand_seed': 5,
    'granularity': 1,
    'min_intermediates': 10,
    'max_intermediates': 10,
    'grain_for_opacity': false,
    // 'bg_color': {'r': 1., 'g': 1., 'b': 1., 'a': 1},
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 5,
      'objects': [
        {'id': 'black_circle', 'x': -260, 'y': 0, 'z': 0},
        {'id': 'outer_circle', 'x': -260, 'y': 0, 'z': 0, 'props': {'direction': 1.}},
        {'id': 'outer_circle', 'x': -260, 'y': 0, 'z': 0, 'props': {'direction': -1.}},
        {'id': 'circles_pattern', 'x': 260, 'y': 0, 'z': 0, 'angle': 180},
        {'id': 'logo_text', 'x': 150, 'y': 25, 'z': 0},
      ]
    },
  ]
};
