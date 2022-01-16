_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 0},
    ],
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
  },
  'objects': {
    'mother': {
      'x': 0,
      'y': 0,
      'subobj': [],
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        this.subobj.push(this.spawn({
          'id': 'ball',
          'props': {'level': 1, 'scale': 1.},
          'x': 0,
          'angle': 0,
          'y': 0,
          'z': 0,
        }));
      },
      'time': function(t, e, s, tt) {},
    },
    'ball': {
      'type': 'circle',
      'opacity': 1,
      'blending_type': blending_type.add,
      'gradient': 'blue',
      'radius': 10.,
      'pivot': true,
      'radiussize': 2.0,
      'props': {'level': 0, 'left': [], 'right': []},
      'init': function() {
        if (this.props.level > 2) {
          return;
        }
        let n = 5;
        let first = false;
        let prev = false;
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
          // opac = 1.;// hack
          let sub = this.spawn({
            'id': 'ball',
            'props': {'level': this.props.level + 1},
            //'opacity': opac,
            // 'angle': angle,
            // 'scale': 1.0 / this.props.level,
            // 'radius': this.props.level,
            'x': new_x,
            'y': new_y,
            'z': 0,
          });

          if (prev) {
            let line = this.spawn({
              'id': 'line',
              // 'scale': 1.0 / this.props.level,
              // 'opacity': opac * 0.5,
              'x': prev.x,
              'y': prev.y,
              'x2': new_x,
              'y2': new_y,
              'z': 0,
            });
            sub.props.left.push(line);
            prev.props.right.push(line);
            this.subobj.push(line);
          }

          this.subobj.push(sub);
          if (!first) first = sub;
          prev = sub;
        }
        if (first && prev) {
          let line = this.spawn({
            'id': 'line',
            // 'scale': 1.0 / this.props.level,
            // 'opacity': opac * 0.5,
            'x': first.x,
            'y': first.y,
            'x2': prev.x,
            'y2': prev.y,
            'z': 0,
          });
          first.props.left.push(line);
          prev.props.right.push(line);
          this.subobj.push(line);
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
        // this.angle = 360 - ((360 * t * this.props.level) / 5.);

        var total_frames = script.video.duration * 25.;
        var current_frame = total_frames * t;
        this.radius = triangular_wave(current_frame, 1., 100) * 50. * this.props.level;
        if (this.radius < 10) this.radius = 10.;
        if (this.radius > 10) {
          let f = this.radius - 10;  // f = number between zero and 40
          f /= 40;                   // f = number between zero and one
          f = 1.0 - f;               // inverted, 1.0 for radius 10,
          this.opacity = expf(f, 100);
        } else {
          this.opacity = 1;
        }
      },
    },
    'line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'green',
      'radius': 0,
      //'opacity': 0.5,  // / 3.,
      'opacity': 0.5,  // / 3.,
      'radiussize': 3,
      'init': function() {},
      'time': function(t) {},
    },
  },
  'video': {
    'duration': 10,
    'fps': 25,
    'width': 3000,
    'height': 3000,
    'scale': 0.8,
    'rand_seed': 23,
    'granularity': 1,
    'grain_for_opacity': true,
    'dithering': true,
    'min_intermediates': 50,
    //'min_intermediates': 2,
    //'max_intermediates': 2,
    'minimize_steps_per_object': false,  // this guy is interesting to debug!!
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'fast_ff': true,
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
    'objects': [
      {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1., 'scale': 1., 'props': {}},
    ],
  }]
};
