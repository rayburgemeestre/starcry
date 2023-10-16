_ = {
  'gradients': {
    'green': '#00ff00@1.0',
  },
  'objects': {
    'sub': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 0,
      'blending_type': blending_type.normal,
      'recursive_scale': 2.,  // scale translates into recursive_scale, eERR, that's only the case for scripts.
      'init': function() {
        // should inherit recursive_scale...
        output('spawning with recursive scale: ' + this.recursive_scale);
        this.spawn({'id': 'point'});
      }
    },
    'point': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 50,
      'gradient': 'green',
      'blending_type': blending_type.normal,
      'init': function() {
        output('spawned with recursive scale: ' + this.recursive_scale);
        output('my init: ' + this.scale + ' & ' + this.recursive_scale);
      }
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 2080,
    'scale': 1.0,
    'rand_seed': 5,
    'granularity': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'grain_for_opacity': false,
  },
  'preview': {
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 3.0,
      'objects': [{'id': 'point', 'x': -100, 'y': 0, 'z': 0, 'scale': 2.}, {'id': 'sub', 'x': 100, 'y': 0, 'z': 0}]
    },
  ]
};
