_ = {
  'gradients': {
    'red': [
      {'position': 0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'obj0': {
      'init': function() {
        this.spawn({'id': 'obj1', 'x': -300, 'y': 0, 'z': 0, 'props': {}});
        this.spawn({'id': 'obj1', 'x': 300, 'y': 0, 'z': 0, 'props': {}});
        this.spawn({'id': 'obj1', 'x': 0, 'y': -300, 'z': 0, 'props': {}});
        this.spawn({'id': 'obj1', 'x': 0, 'y': +300, 'z': 0, 'props': {}});
      },
      'time': function(t, e, scene) {
        if (scene === 1) this.angle = -360. * expf(t, 10.);
      },
    },
    'obj1': {
      'init': function() {
        this.spawn({'id': 'obj2', 'x': -100, 'y': 0, 'z': 0, 'props': {'maxradius': 100}});
        this.spawn({'id': 'obj2', 'x': 0, 'y': 0, 'z': 0, 'props': {'maxradius': 200}});
        this.spawn({'id': 'obj2', 'x': 100, 'y': 0, 'z': 0, 'props': {'maxradius': 300}});
      },
    },
    'obj2': {
      'type': 'circle',
      'radius': 0,
      'hue': 180.0,
      'radiussize': 10.0,
      'gradients': [[1.0, 'red']],
      'opacity': 1.,
      'props': {
        'maxradius': 250.0,
      },
      'blending_type': blending_type.normal,
      'init': function() {},
      'time': function(t, elapsed, s) {
        this.hue += elapsed * 200;
        if (s <= 1) {
          this.radius += 100.0 * elapsed;
          this.radius %= this.props.maxradius;
        } else {
          this.radius = logn(t, 1.) * 1920.0;
        }
      },
    },
  },
  'video': {
    'bg_color': {'r': 1., 'g': 1., 'b': 1., 'a': 1},
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 3.0,
      'objects': [
        {'id': 'obj0', 'x': 5, 'y': 0, 'z': 0, 'props': {}},
        {'id': 'obj0', 'x': -5, 'y': 0, 'z': 0, 'props': {}},
      ]
    },
    {'name': 'scene2', 'duration': 1.0, 'objects': []},
    {'name': 'scene3', 'duration': 1.0, 'objects': []},
  ]
};
