_ = {
  'gradients': {
    'full_white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.90, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
  },
  'textures': {},
  'objects': {
    'balls': {
      'x': 0,
      'y': 0,
      'props': {},
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        this.spawn({
          'id': 'ball',
          'label': 'ball_1',
          'x': -100,
          'y': 0,
          'z': 0,
          'velocity': 10,  // do not specify negative velocity
          'vel_x': 1,
          'vel_y': 0,
        });
        this.spawn({
          'id': 'ball',
          'label': 'ball_2',
          'x': 100,
          'y': 0,
          'z': 0,
          'velocity': 10,  // do not specify negative velocity
          'vel_x': -1,
          'vel_y': 0,
        });
      },
      'time': function(t, e) {},
    },
    'ball': {
      'collision_group': 'group1',
      'type': 'circle',
      'gradient': 'full_white',
      'radius': 0,
      'radiussize': 20.0,
      'init': function() {},
      'time': function(t, elapsed) {},
      'collide': function(other) {
        this.spawn({'id': 'tempring', 'label': 'ring_' + this.label, 'x': 0, 'y': 0, 'z': 0, 'props': {}});
      },
    },
    'tempring': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 5.0,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {
        this.radius += elapsed * 50;
        if (this.radius >= 200.) {
          this.exists = false;
        }
      },
    },
    'bg': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 0,
      'init': function() {
        this.seed = rand();
      },
      'time': function(t, elapsed) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.,
    'rand_seed': 1,
    'granularity': 1,
    'grain_for_opacity': false,
    'dithering': false,
    //'motion_blur': false, <<< TODO: this causes issues! (object disappears!)
    'max_intermediates': 3,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 1,
    'objects': [
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
