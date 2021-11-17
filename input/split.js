_ = {
  'gradients': {
    'white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'splitter': {
      'type': 'circle',
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 0,
      'angle': 0.,
      'init': function() {
        this.subobj.push({'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'props': {'x': 0, 'y': 0}});
        this.subobj.push({'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'props': {'x': 0, 'y': 0}});
      },
      'time': function(t, e, scene) {
        let jump = 100;
        if (scene == 0) {
          this.subobj[0].props.x = expf(t, 50) * jump;
          this.subobj[1].props.x = expf(t, 50) * -jump;
        } else {
          this.subobj[0].props.x = jump;
          this.subobj[1].props.x = -jump;
        }
      },
    },
    'circle': {
      'type': 'circle',
      'gradient': 'white',
      'x': -50,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 50.0,
      'angle': 0.,
      'init': function() {},
      'time': function(t, e, scene) {
        let jump = 100;
        var total_frames = 0.5 /*scene duration*/ * 25.;
        var current_frame = total_frames * t;
        switch (scene) {
          case 0:
            this.x = this.props.x + triangular_wave(current_frame, 1., 1.0) * (t * jump);
            break;
          case 1:
            this.x = this.props.x;
            break;
        }
      },
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 5,
    'granularity': 1,
    'min_intermediates': 5.,
    'max_intermediates': 50.,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 0},
    'grain_for_opacity': false,
  },
  'preview': {
    'max_intermediates': 1.,
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 0.5,
      'objects': [
        {'id': 'circle', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},
        {'id': 'circle', 'x': rand() * 1000 - 500, 'y': rand() * 1000 - 500, 'z': 0, 'props': {}},

      ]
    },
    {'name': 'scene2', 'duration': 0.5, 'objects': []},
  ]
};
