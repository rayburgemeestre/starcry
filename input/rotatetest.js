_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
  },
  'toroidal': {},
  'objects': {
    'ball': {
      'type': 'circle',
      'radius': 10,
      'radiussize': 10.0,
      'init': function() {
        if (this.props.mode === 'line_up') {
          this.subobj.push(this.spawn({'id': 'line', 'x': 0, 'y': 0, 'x2': 0, 'y2': -100}));
        } else {
          // this.subobj.push(this.spawn({'id': 'line', 'x': 0, 'y': 0, 'x2': 0, 'y2': -200}));
        }

        // add a ball as well
        // this.subobj.push(this.spawn({'id': 'ball', 'x': 0, 'y': 0, 'x2': 0, 'y2': -100}));
      },
      'time': function(t) {},
    },
    'line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 2,
      'init': function() {},
      'time': function(t, e) {
        this.angle = 360 * t;
      },
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1.0,
    'granularity': 1,
    'min_intermediates': 5,
    'max_intermediates': 5,
    'minimize_steps_per_object': false,  // this guy is interesting to debug!!
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    'motion_blur': false,
    'grain_for_opacity': false,
    'dithering': false,
    'width': 1920,
    'height': 1080,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 10,
    'objects': [
      {'id': 'ball', 'x': 0, 'y': 0, 'z': 0, 'props': {'mode': 'line_up'}},
      {'id': 'ball', 'x': 100, 'y': 0, 'z': 0, 'props': {'mode': 'line_up'}},
      {'id': 'ball', 'x': 200, 'y': 0, 'z': 0, 'props': {'mode': 'line_up'}},
    ],
  }]
};
