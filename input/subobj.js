_ = {
  'gradients': {},
  'textures': {},
  'objects': {
    'balls': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        this.subobj.push({
          'id': 'ball',
          'label': 'ball_1',
          'x': -100,
          'y': 0,
          'z': 0,
          'velocity': 10,
          'vel_x': 1,
          'vel_y': 0,
        });
        this.subobj.push({
          'id': 'ball',
          'label': 'ball_2',
          'x': 100,
          'y': 0,
          'z': 0,
          'velocity': -10,
          'vel_x': 1,
          'vel_y': 0,
        });
      },
      'time': function(t, e) {},
    },
    'ball': {
      'collision_group': 'group1',
      'type': 'circle',
      'gradients': [],
      'radius': 0,
      'radiussize': 20.0,
      'props': {'grad': 'white'},
      'init': function() {},
      'time': function(t, elapsed) {},
      'subobj': [],
      'on': {
        'collide': function(other) {
          this.subobj.push({'id': 'tempring', 'label': 'ring_' + this.label, 'x': 0, 'y': 0, 'z': 0, 'props': {}});
        }
      }
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
    'duration': 5,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.,
    'rand_seed': 1,
    'granularity': 1,
    'grain_for_opacity': false,
    'dithering': false,
    'motion_blur': false,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
