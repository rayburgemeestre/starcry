script = {
  'gradients': {},
  'objects': {
    'main': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 0.0,
      'angle': 0.,
      'init': function() {
        let canvas_size = 2000;
        for (let i = 0; i < 200; i++) {
          let x = (rand() * canvas_size) - canvas_size / 2;
          let y = (rand() * canvas_size) - canvas_size / 2;
          this.spawn({'id': 'point', 'x': x, 'y': y, 'z': 0, 'props': {}});
        }
      },
      'time': function(t, e, scene) {},
    },
    'point': {
      'x': 0,
      'y': 0,
      'type': 'circle',
      'radius': 0,
      'radiussize': 20,
      'init': function() {},
    },

  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [
    {'name': 'scene1', 'duration': 60.0, 'objects': [{'id': 'main', 'x': 5, 'y': 0, 'z': 0, 'props': {}}]},
  ]
}
