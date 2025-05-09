script = {
  'gradients': {
    'black': [
      {'position': 0, 'r': 0, 'g': 0, 'b': 0, 'a': 1},
      /*
    {
      "position": 0.50,
      "r": 0,
      "g": 0,
      "b": 0,
      "a": 1
    },
    */
      {'position': 1, 'r': 0, 'g': 0, 'b': 0, 'a': 0}
    ],
    'white': [
      {'position': 0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0}
    ],
    'green': [
      {'position': 0, 'r': 1, 'g': 255, 'b': 1, 'a': 1},
      {'position': 0.5, 'r': 0, 'g': 1, 'b': 0, 'a': 0.5},
      {'position': 1, 'r': 0, 'g': 255, 'b': 0, 'a': 0}
    ],
    'blue': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.4, 'r': 0.5, 'g': 0, 'b': 101, 'a': 1},
      {'position': 0.65, 'r': 0, 'g': 0, 'b': 1, 'a': 0.8},
      {'position': 1, 'r': 0, 'g': 0, 'b': 0, 'a': 0}
    ]
  },
  'toroidal': {'t1': {'width': 1920, 'height': 1080}},
  'objects': {
    'mother': {
      'type': 'circle',
      'x': 0,
      'y': 0,
      'subobj': [],
      'radius': 0,
      'radiussize': 0,
      'init': function() {
        this.spawn({
          'id': 'ball',
          'props': {'level': 1, 'scale': 1.},
          'x': 0,
          'angle': 0,
          'y': 0,
          'z': 0,
          'gradient': this.gradient,
        });
      },
      'time': function(t, e, s, tt) {
        var total_frames = script.video.duration * 25.;
        var current_frame = total_frames * t;
        // this.scale = 2 + triangular_wave(current_frame, 1., 0.5) * 0.10;
      }
    },
    'black': {
      'type': 'circle',
      'opacity': 1,
      'blending_type': blending_type.normal,
      'gradient': 'black',
      'radius': 0,
      'radiussize': 200
    },
    'ball': {
      'type': 'circle',
      'opacity': 0,
      'blending_type': blending_type.normal,
      'gradient': 'white',
      'radius': 0,
      'radiussize': 0,
      'props': {'level': 0},
      'init': function() {
        // if (this.props.level === 1) {
        //   let sub = this.spawn({
        //     "id": "ball",
        //     "props": {"level": this.props.level + 1},
        //     //"opacity": opac,
        //     // "angle": angle,
        //     // "scale": 1.0 / this.props.level,
        //     "x": 100,
        //     "y": 0,
        //     "z": 0,
        //     "gradient": this.gradient,
        //   });
        // }
        // if (this.props.level === 2) {
        //   let sub = this.spawn({
        //     "id": "ball",
        //     "props": {"level": this.props.level + 1},
        //     //"opacity": opac,
        //     // "angle": angle,
        //     // "scale": 1.0 / this.props.level,
        //     "x": 100,
        //     "y": 0,
        //     "z": 0,
        //     "gradient": this.gradient,
        //   });
        // }
        // if (this.props.level === 3) {
        //   let sub = this.spawn({
        //     "id": "ball",
        //     "props": {"level": this.props.level + 1},
        //     //"opacity": opac,
        //     // "angle": angle,
        //     // "scale": 1.0 / this.props.level,
        //     "x": 100,
        //     "y": 0,
        //     "z": 0,
        //     "gradient": this.gradient,
        //   });
        // }
        // return;

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
            //"opacity": opac,
            // "angle": angle,
            // "scale": 1.0 / this.props.level,
            'x': new_x,
            'y': new_y,
            'z': 0,
            'gradient': this.gradient,
          });

          this.spawn2(
              {
                'id': 'line',
                // "scale": 1.0 / this.props.level,
                // "opacity": opac * 0.5,
                'z': 0,
                'gradient': 'blue',
              },
              sub);
        }
      },
      'time': function(t, e, s, tt) {
        // nice
        // if (this.level == 2) {
        //  this.angle = (360. * t) / 5.;
        // }
        // nice but too fast
        this.angle = 360. * (t * this.props.level);
        // third try
        // this.angle = 360 * ((t * this.props.level) / 5.);
        // this.angle = (360 * t * this.props.level) / 5.;

        // for (var i of this.props.left) {
        //   i.x = this.x;
        //   i.y = this.y;
        // }
        // for (var i of this.props.right) {
        //   i.x2 = this.x;
        //   i.y2 = this.y;
        // }
      }
    },
    'line': {
      'type': 'line',
      'blending_type': blending_type.add,
      'gradient': 'green',
      'radius': 0,
      'opacity': 1,
      'radiussize': 1,
      'init': function() {},
      'time': function(t) {}
    }
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 1.2,
    'rand_seed': 23,
    'granularity': 1,
    'grain_for_opacity': true,
    'dithering': true,
    'min_intermediates': 20,
    'max_intermediates': 20,
    'minimize_steps_per_object': false,
    'gamma': 0.2,
    'brightness': 5,
    'bg_color': {'r': 0, 'g': 0, 'b': 0, 'a': 1}
  },
  'preview': {
    'motion_blur': false,
    'grain_for_opacity': false,
    'dithering': false,
    'min_intermediates': 20,
    'max_intermediates': 20,
    'width': 300,
    'height': 300
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 30,
    'objects': [
      {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'opacity': 1, 'scale': 1, 'props': {}, 'gradient': 'blue'},
      {
        'id': 'mother',
        'x': 0,
        'y': 0,
        'z': 0,
        'opacity': 1,
        'scale': 2,
        'rotate': 10,
        'props': {},
        'gradient': 'white'
      },
      {
        'id': 'mother',
        'x': 0,
        'y': 0,
        'z': 0,
        'opacity': 1,
        'scale': 3,
        'rotate': 20,
        'props': {},
        'gradient': 'green'
      },
      {'id': 'black', 'opacity': 1, 'scale': 1.0},
      {'id': 'black', 'opacity': 1, 'scale': 0.6}

    ]
  }],
  'textures': {}
}
