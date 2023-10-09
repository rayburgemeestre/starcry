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
    'triangles': {
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        let n = 3;
        let subobj = [];
        for (let i = 0; i < n; i++)
          subobj.push(this.spawn({
            'id': 'ball',
            'x': 0,
            'y': 0,
            'z': 0,
            'velocity': rand() * 100.,
            'vel_x': ((rand() * 2.) - 1.),
            'vel_y': ((rand() * 2.) - 1.),
          }));

        for (let i = 0; i < n; i++) {
          for (let j = 0; j < n; j++) {
            let o1 = subobj[i];
            let o2 = subobj[j];
            let line = this.spawn3(
                {
                  'id': 'line',
                  'x': o1.x,
                  'y': o1.y,
                  'x2': o2.x,
                  'y2': o2.y,
                  'z': 0,
                },
                o1,
                o2);
          }
        }
      },
      'time': function(t) {
        this.angle = 360. * t;
      },
    },
    'ball': {
      'type': 'circle',
      'opacity': 0,
      'toroidal': 't1',
      'blending_type': blending_type.normal,
      'gradient': 'white',
      'radius': 10,
      'radiussize': 10.0,
      'props': {'left': [], 'right': []},
      'init': function() {
        this.props.seed = rand();
        this.props.vel = this.velocity;
      },
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
      'time': function(t) {},
    },
  },
  'video': {
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
    'duration': 10,
    'objects': [
      {'id': 'triangles', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1., 'scale': 1., 'props': {}},
    ],
  }]
};