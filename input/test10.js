_ = {
  'gradients': {
    'red': '#ff0000',
  },
  'objects': {
    'obj1': {
      'type': 'circle',
      'hue': 180.0,
      'gradient': 'red',
      'radius': 100.0,
      'radiussize': 10.0,
      'time': function(t, elapsed, s, tt) {
        output('elapsed: ' + elapsed + ', and t:' + t);
        this.hue = 360.0 * t;
      },
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.0,
    'rand_seed': 5,
    'granularity': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'grain_for_opacity': false,
  },
  'scenes': [
    {'name': 'scene1', 'duration': 3.0, 'objects': [{'id': 'obj1', 'x': 0, 'y': 0, 'z': 0, 'props': {}}]},
  ]
};
