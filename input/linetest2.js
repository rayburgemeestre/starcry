_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
    'green': [
      //{'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.0, 'r': 69 / 255., 'g': 252 / 255., 'b': 157 / 255., 'a': 1},
      {'position': 0.52, 'r': 69 / 255., 'g': 252 / 255., 'b': 157 / 255., 'a': 1},
      {'position': 1.0, 'r': 82, 'g': 255, 'b': 101, 'a': 0},
    ],
  },
  'toroidal': {
    't1': {
      'width': 1920,
      'height': 1080,
    }
  },
  'objects': {
    'mother': {
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        this.spawn({
          'id': 'ball',
          'props': {'level': 1},
          'x': 0,
          'angle': 0,
          'y': 0,
          'z': 0,
        });
      },
      'time': function(t, e, s, tt) {
        this.angle = 360. * t;
        this.scale += e * 20;
      },
    },
    'ball': {
      'type': 'circle',
      'opacity': 0,
      'blending_type': blending_type.normal,
      'gradient': 'white',
      'radius': 10,
      'radiussize': 10.0,
      'props': {'level': 0},
      'init': function() {
        if (this.props.level > 4) {
          return;
        }
        let n = Math.round(rand() * 10);
        for (var a = 0; a < n; a++) {
          var angle = a * 360 / n;
          if (this.angle) angle += this.angle;
          while (angle > 360) angle -= 360;
          var rads = angle * Math.PI / 180.0;
          var move = 400 / this.props.level;
          var new_x = (Math.cos(rads) * move);
          var new_y = (Math.sin(rads) * move);

          // let opac = this.opacity / this.level;
          let opac = 0.25 * (this.level - 1);
          if (opac > 1.) opac = 1.;
          if (opac < 0.) opac = 0.;
          this.spawn({
            'id': 'ball',
            'props': {'level': this.props.level + 1},
            //'opacity': opac,
            // 'angle': angle,
            // 'scale': 1.0 / this.props.level,
            'x': new_x,
            'y': new_y,
            'z': 0,
          });

          this.spawn({
            'id': 'line',
            // 'scale': 1.0 / this.props.level,
            'opacity': opac * 0.5,
            'x': 0,
            'y': 0,
            'x2': new_x,
            'y2': new_y,
            'z': 0,
          });
        }
      },
      'time': function(t) {},
    },
    'line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'green',
      'radius': 0,
      'opacity': 1. / 3.,
      'radiussize': 2,
      'init': function() {},
      'time': function(t) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 5000,
    'height': 5000,
    'scale': 0.66,
    'rand_seed': 23,
    'granularity': 1,
    'grain_for_opacity': true,
    'dithering': true,
    'min_intermediates': 20,
    'max_intermediates': 20,
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
      {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1., 'scale': 1., 'props': {}},
    ],
  }]
};
