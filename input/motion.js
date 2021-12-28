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
    'balls': {
      'x': 0,
      'y': 0,
      'subobj': [],
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        let n = 10;
        for (let i = 0; i < n; i++)
          this.subobj.push({
            'id': 'ball',
            //'x': ((rand() * 2.) - 1.) * 1920/2,
            //'y': ((rand() * 2.) - 1.) * 1920/2,
            'x': 0,
            'y': 0,
            'z': 0,
            'velocity': rand() * 100.,
            'vel_x': ((rand() * 2.) - 1.),
            'vel_y': ((rand() * 2.) - 1.),
          });
        for (let i = 0; i < n; i++) {
          for (let j = 0; j < n; j++) {
            if (i <= j) {
              let obj1 = this.subobj[i];
              let obj2 = this.subobj[j];
              this.subobj.push({
                'id': 'line',
                'opacity': rand(),
                'x': obj1.x,
                'y': obj1.y,
                'x2': obj2.x,
                'y2': obj2.y,
                'z': 0,
              });
              // obj1.props.left = this.subobj[this.subobj.length - 1];
              // obj2.props.right = this.subobj[this.subobj.length - 1];
            }
          }
        }
      },
      'time': function(t) {
        let n = 10;
        let index = 0;
        for (let i = 0; i < n; i++) {
          for (let j = 0; j < n; j++) {
            if (i <= j) {
              let obj1 = this.subobj[i];
              let obj2 = this.subobj[j];

              while (this.subobj[index].id != 'line') index++;

              this.subobj[index].x = obj1.x;
              this.subobj[index].y = obj1.y;
              this.subobj[index].x2 = obj2.x;
              this.subobj[index].y2 = obj2.y;

              obj1.props.left = this.subobj[index];
              obj2.props.right = this.subobj[index];
              index++;
            }
          }
        }
        this.angle = 360. * t;
      },
    },
    'ball': {
      'type': 'circle',
      'opacity': 0,
      // 'collision_group': 'cg1',
      //      'toroidal': 't1',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 10,
      'radiussize': 10.0,
      'props': {'left': false, 'right': false},
      'init': function() {
        this.props.seed = rand();
        this.props.vel = this.velocity;
      },
      'time': function(t) {
        if (this.props.left !== false) {
          this.props.left.x = this.x;
          this.props.left.y = this.y;
        }
        if (this.props.right !== false) {
          this.props.right.x2 = this.x;
          this.props.right.y2 = this.y;
        }
        this.velocity = this.props.vel * Math.sin(t * 100 * this.props.seed);
      },
    },
    'line': {
      'type': 'line',
      // 'collision_group': 'cg1',
      // 'toroidal': 't1',
      'blending_type': blending_type.add,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 2,
      'init': function() {},
      'time': function(t) {},
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
    'min_intermediates': 30,
    'max_intermediates': 30,
    //'max_intermediates': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    'motion_blur': false,
    'min_intermediates': 1,
    'max_intermediates': 1,
    'grain_for_opacity': false,
    'dithering': false,
    'width': 1920,
    'height': 1080,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 3., 'scale': 1., 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 3., 'scale': 1., 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 3., 'scale': 1., 'props': {}},

      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 6., 'scale': 2., 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 7., 'scale': 3., 'props': {}},
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1. / 8., 'scale': 4., 'props': {}},
    ],
  }]
};
