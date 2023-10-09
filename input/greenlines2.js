_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
    'green': [
      {'position': 0.0, 'r': 0, 'g': 255, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 255, 'b': 0, 'a': 0},
      // {'position': 0.0, 'r': 82, 'g': 255, 'b': 101, 'a': 1},
      // {'position': 1.0, 'r': 82, 'g': 255, 'b': 101, 'a': 0},
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
          'props': {'level': 1, 'scale': 1.},
          'x': 0,
          'angle': 0,
          'y': 0,
          'z': 0,
        });
      },
      'time': function(t, e, s, tt) {
        var total_frames = script.video.duration * 25.;
        var current_frame = total_frames * t;
        // this.scale = 2 + triangular_wave(current_frame, 1., 0.5) * 0.10;
      },
    },
    'ball': {
      'type': 'circle',
      'opacity': 0,
      'blending_type': blending_type.normal,
      'gradient': 'white',
      'radius': 10.,
      'pivot': true,
      'radiussize': 10.0,
      'props': {'level': 0, 'left': [], 'right': []},
      'init': function() {
        if (this.props.level > 3) {
          return;
        }
        let n = 12;  // Math.round(rand() * 10);
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
          let sub = this.spawn({
            'id': 'ball',
            'props': {'level': this.props.level + 1},
            'opacity': opac,
            // 'angle': angle,
            // 'scale': 1.0 / this.props.level,
            'x': new_x,
            'y': new_y,
            'z': 0,
          });

          let line = this.spawn3(
              {
                'id': 'line',
                // 'scale': 1.0 / this.props.level,
                'opacity': opac * 0.5,
                'x': 0,
                'y': 0,
                'x2': new_x,
                'y2': new_y,
                'z': 0,
              },
              sub,
              this.unique_id);
        }
      },
      'time': function(t, e, s, tt) {
        // nice
        // if (this.level == 2) {
        //  this.angle = (360. * t) / 5.;
        // }
        // nice but too fast
        // this.angle = 360. * (t * this.props.level);
        // third try
        // this.angle = 360 * ((t * this.props.level) / 5.);
        this.angle = (360 * t * this.props.level) / 5.;

        // for (var i of this.props.left) {
        //   i.x = this.x;
        //   i.y = this.y;
        // }
        // for (var i of this.props.right) {
        //   i.x2 = this.x;
        //   i.y2 = this.y;
        // }
      },
    },
    'line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'green',
      'radius': 0,
      //'opacity': 0.5,  // / 3.,
      'opacity': 0.5,  // / 3.,
      'radiussize': 2,
      'init': function() {},
      'time': function(t) {},
    },
  },
  'video': {
    'fps': 25,
    'width': 3000,
    'height': 3000,
    'scale': 0.8,
    'rand_seed': 23,
    'granularity': 1,
    'grain_for_opacity': false,
    'dithering': true,
    'min_intermediates': 3,
    //'max_intermediates': 2,
    'minimize_steps_per_object': false,  // this guy is interesting to debug!!
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'preview': {
    'motion_blur': false,
    'grain_for_opacity': false,
    'dithering': false,
    'min_intermediates': 2,
    'max_intermediates': 2,
    'width': 300,
    'height': 300,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 10,
    'objects': [
      {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1., 'scale': 1., 'props': {}},
    ],
  }]
};
