_ = {
  'gradients': {
    'ce475a6c-2427-420c-85de-6316f3027313': [
      {'position': 0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'cd35b59b-bae9-4bef-b689-dcc8720e6be9': [
      {'position': 0, 'r': 0, 'g': 1, 'b': 0, 'a': 1},
      {'position': 1, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    '64f1da9f-07b2-46bd-8f62-30f9534f0cfd': [
      {'position': 0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ]
  },
  'objects': {
    'obj0': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 5.0,
      'angle': 0.,
      'init': function() {
        this.subobj.push({'id': 'obj1', 'x': -300, 'y': 0, 'z': 0, 'props': {}});
        this.subobj.push({'id': 'obj1', 'x': 300, 'y': 0, 'z': 0, 'props': {}});
        this.subobj.push({'id': 'obj1', 'x': 0, 'y': -300, 'z': 0, 'props': {}});
        this.subobj.push({'id': 'obj1', 'x': 0, 'y': +300, 'z': 0, 'props': {}});
      },
      'time': function(t, e, scene) {
        switch (scene) {
          case 0:
            this.angle = 360. * expf(t, 1.);
            break;
          case 1:
            this.angle = -360. * expf(t, 10.);
            break;
        }
      },
    },
    'obj1': {
      'x': 0,
      'y': 0,
      'props': {
        'maxradius': 250.0,
      },
      'gradients': [
        [1.0, 'ce475a6c-2427-420c-85de-6316f3027313'],
        [0.0, 'cd35b59b-bae9-4bef-b689-dcc8720e6be9'],
        [0.0, '64f1da9f-07b2-46bd-8f62-30f9534f0cfd'],
      ],
      'subobj': [],
      'radius': 0,
      'radiussize': 5.0,
      'init': function() {
        this.subobj.push({'id': 'obj2', 'x': -100, 'y': 0, 'z': 0, 'props': {'maxradius': 100}});
        this.subobj.push({'id': 'obj2', 'x': 0, 'y': 0, 'z': 0, 'props': {'maxradius': 200}});
        this.subobj.push({'id': 'obj2', 'x': 100, 'y': 0, 'z': 0, 'props': {'maxradius': 300}});
      },
      'time': function(t, e, s) {
        switch (s) {
          case 0:
            this.radius = t * this.props.maxradius;
            break;
          case 1:
            this.radius = t * this.props.maxradius * 2.0;
            break;
        }
        this.gradients[0][0] = 1.0 - t;
        this.gradients[2][0] = t;
      },
      'proximity': function(t) {}
    },
    'obj2': {
      'type': 'circle',
      'radius': 100,
      'radiussize': 10.0,
      'props': {
        'maxradius': 250.0,
      },
      'init': function() {
        this.radius = 0;
      },
      'time': function(t, elapsed, s) {
        switch (s) {
          case 0:
          case 1:
            this.radius += 100.0 * elapsed;
            this.radius %= this.props.maxradius;
            break;
          case 2:
          case 3:
          case 4:
            this.radius = logn(t, 10.) * 1920.0;
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
    'max_intermediates': 50.,
    'bg_color': {'r': 0., 'g': 0.0, 'b': 0., 'a': 0}
  },
  'scenes': [
    {'name': 'scene1', 'duration': 1.0, 'objects': [{'id': 'obj0', 'x': 0, 'y': 0, 'z': 0, 'props': {}}]},
    {'name': 'scene2', 'duration': 1.0, 'objects': [{'id': 'obj0', 'x': 100, 'y': 0, 'z': 0, 'props': {}}]},
    //    {'name': 'scene3', 'duration': 1.0, 'objects': [{'id': 'obj0', 'x': -100, 'y': 0, 'z': 0, 'props': {}}]},
    //    {'name': 'scene4', 'duration': 1.0, 'objects': [{'id': 'obj0', 'x': -100, 'y': 0, 'z': 0, 'props': {}}]},
    //    {'name': 'scene5', 'duration': 1.0, 'objects': [{'id': 'obj0', 'x': -100, 'y': 0, 'z': 0, 'props': {}}]},
  ]
};
