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
        this.subobj.push({
          'id': 'ball',
          'x': -50,
          'y': 0,
          'z': 0,
          'velocity': rand() * 100.,
          'vel_x': ((rand() * 2.) - 1.),
          'vel_y': ((rand() * 2.) - 1.)
        });
        this.subobj.push({
          'id': 'ball',
          'x': 50,
          'y': 0,
          'z': 0,
          'velocity': rand() * 100.,
          'vel_x': ((rand() * 2.) - 1.),
          'vel_y': ((rand() * 2.) - 1.)
        });
        this.subobj.push({'id': 'line', 'x': -50, 'y': 0, 'x2': 50, 'y2': 0, 'z': 0});
      },
      'init2': function() {
        if (this.subobj[0].props.left.length == 0) {
          output(
              'init2 connecting to first index (' + this.subobj[0].unique_id + ') left (' + this.subobj[2].unique_id +
              ')');
          this.subobj[0].props.left.push(this.subobj[2]);
        }
        if (this.subobj[1].props.right.length == 0) {
          output(
              'init2 connecting to second index (' + this.subobj[1].unique_id + ') right (' + this.subobj[2].unique_id +
              ')');
          this.subobj[1].props.right.push(this.subobj[2]);
        }
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
        // if (this.props.left !== false) {
        //   this.props.left.x = this.x;
        //   this.props.left.y = this.y;
        // }
        // if (this.props.right !== false) {
        //   this.props.right.x2 = this.x;
        //   this.props.right.y2 = this.y;
        // }
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
    'duration': 10,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 3,
    'granularity': 1,
    'grain_for_opacity': true,
    'dithering': true,
    'min_intermediates': 2,
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