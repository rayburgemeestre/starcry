_ = {
  'gradients': {
    'white': '#ffffff@0.0',
    'red': '#ff0000@0.0',
  },
  'objects': {
    'circle': {
      'type': 'circle',
      'gradient': 'white',
      'x': 0,
      'y': 0,
      'radius': 100,
      'radiussize': 2.0,
      'angle': 0.,
    },
    'ellipse': {
      'type': 'ellipse',
      'gradient': 'red',
      'x': 0,
      'y': 0,
      'shortest_diameter': 100,
      'longest_diameter': 70,
      'radiussize': 20.0,
      'rotate': -45,
      'time': function(t, e) {
        this.rotate += e * 50.;
      }
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
  'preview': {
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 3.0,
      'objects': [
        {
          'id': 'ellipse',
          'x': 0,
          'y': 0,
          'z': 0,
          'shortest_diameter': attr('shortest_diameter') || 70,
          'longest_diameter': attr('longest_diameter') || 100,
          'radiussize': attr('radiussize') || 20.0,
          'rotate': attr('rotate') || 45
        },
        {
          'id': 'circle',
          'x': 0,
          'y': 0,
          'z': 0,
          'radius': (attr('longest_diameter') || 100) + (attr('radiussize') || 20.0)
        },
        {
          'id': 'circle',
          'x': 0,
          'y': 0,
          'z': 0,
          'radius': (attr('shortest_diameter') || 70) - (attr('radiussize') || 20.0)
        },
      ]
    },
  ]
};
