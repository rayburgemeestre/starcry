_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
  },
  'toroidal': {
    't1': {
      'width': 1920,
      'height': 1080,
    }
  },
  'objects': {
    'test': {
      'x': 0,
      'y': 0,
      'subobj': [],
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        let obj1 = this.spawn({
          'id': 'ball',
          'x': -50,
          'y': 0,
          'z': 0,
          'velocity': rand() * 100.,
          'vel_x': ((rand() * 2.) - 1.),
          'vel_y': ((rand() * 2.) - 1.)
        });
        let obj2 = this.spawn({
          'id': 'ball',
          'x': 50,
          'y': 0,
          'z': 0,
          'velocity': rand() * 100.,
          'vel_x': ((rand() * 2.) - 1.),
          'vel_y': ((rand() * 2.) - 1.)
        });

        let line = this.spawn({'id': 'line', 'x': -50, 'y': 0, 'x2': 50, 'y2': 0, 'z': 0});
        obj1.props.left.push(line);
        obj2.props.right.push(line);
        this.subobj.push(obj1);
        this.subobj.push(obj2);
        this.subobj.push(line);
      },
      'time': function(t) {}
    },
    'ball': {
      'type': 'circle',
      'opacity': 1,
      'toroidal': 't1',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 10,
      'radiussize': 10.0,
      'props': {'left': [], 'right': []},
      'init': function() {},
      'time': function(t) {
        for (var i of this.props.left) {
          i.x = this.x;
          i.y = this.y;
        }
        for (var i of this.props.right) {
          i.x2 = this.x;
          i.y2 = this.y;
        }
      },
    },
    'line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 2,
      'init': function() {},
      'time': function(t, e, s, tt) {
        this.scale = 1. * tt;
      },
    },
  },
  'video': {
    'duration': 10,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 3,
    'granularity': 1,
    'grain_for_opacity': true,
    'dithering': true,
    'minimize_steps_per_object': true,  // this guy is interesting to debug!!
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
    'objects': [
      {'id': 'test', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1., 'scale': 1., 'props': {}},
    ],
  }]
};