_ = {
  'gradients': {
    'mix': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.1, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.11, 'r': 1, 'g': 0, 'b': 0, 'a': 0.5},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'red': [
      {'position': 0.0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'green': [
      {'position': 0.0, 'r': 0, 'g': 1, 'b': 0, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    'blue': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ],
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'mother': {
      'subobj': [],
      'x': 0,
      'y': 0,
      'angle': 0,
      'opacity': 1,
      'props': {},
      'init': function() {
        function key(obj) {
          if (Math.floor(obj[0]) < Math.floor(obj[2]) || Math.floor(obj[1]) < Math.floor(obj[3])) {
            return [Math.floor(obj[0]), Math.floor(obj[1]), Math.floor(obj[2]), Math.floor(obj[3])].join('_');
          } else {
            return [Math.floor(obj[2]), Math.floor(obj[3]), Math.floor(obj[0]), Math.floor(obj[1])].join('_');
          }
          return s;
        }

        var queue = [[0, 0]];
        var existing_line = new Set();


        function line_exists(inp) {
          var k = key(inp);
          if (existing_line.has(inp)) {
            return true;
          }
          return false;
        }

        while (queue.length > 0) {
          var current = queue.shift();

          if (get_distance(current[0], current[1], 0, 0) > 190.) {
            break;
          }

          // create four lines and add them to queue
          if (!line_exists(key([current[0] + 100, current[1] + 0, current[0], current[1]]))) {
            this.subobj.push({
              'id': 'test_line',
              'x': current[0] + 100.,
              'y': current[1] + 0,
              'x2': current[0],
              'y2': current[1],
              'props': {}
            });
            queue.push([current[0] + 100, current[1] + 0]);
            existing_line.add(key([current[0] + 100, current[1] + 0, current[0], current[1]]));
          }
          if (!line_exists(key([current[0] + -100, current[1] + 0, current[0], current[1]]))) {
            this.subobj.push({
              'id': 'test_line',
              'x': current[0] + -100.,
              'y': current[1] + 0,
              'x2': current[0],
              'y2': current[1],
              'props': {}
            });
            queue.push([current[0] + -100, current[1] + 0]);
            existing_line.add(key([current[0] + -100, current[1] + 0, current[0], current[1]]));
          }
          if (!line_exists(key([current[0] + 0, current[1] + 100, current[0], current[1]]))) {
            this.subobj.push({
              'id': 'test_line',
              'x': current[0] + 0.,
              'y': current[1] + 100,
              'x2': current[0],
              'y2': current[1],
              'props': {}
            });
            queue.push([current[0] + 0, current[1] + 100]);
            existing_line.add(key([current[0] + 0, current[1] + 100, current[0], current[1]]));
          }
          if (!line_exists(key([current[0] + 0, current[1] + -100, current[0], current[1]]))) {
            this.subobj.push({
              'id': 'test_line',
              'x': current[0] + 0.,
              'y': current[1] + -100,
              'x2': current[0],
              'y2': current[1],
              'props': {}
            });
            queue.push([current[0] + 0, current[1] + -100]);
            existing_line.add(key([current[0] + 0, current[1] + -100, current[0], current[1]]));
          }
        }
      },
      'time': function(t, e, s) {
        script.video.scale = 1.0 + t * 5;
        if (this.props.scale) {
          // this.scale = 1.0 + t * 1;
          // doesn't work?? this.scale += e * 100.;
        }
      }
    },
    'test_line': {
      'type': 'line',
      'gradient': 'white',
      'x': 0,
      'y': 0,
      'x2': 0,
      'y2': 0,
      'radius': 0.,
      'radiussize': 5.0,
      'opacity': 0.1,
      'blending_type': blending_type.normal,
      'angle': 0,
      'props': {},
      'scale': 1.0,
      'init': function() {
        let s = 0;

        this.subobj.push({
          'id': 'test_line2',
          'x': -s,
          'y': 0,
          'x2': -s,
          'y2': 0,
          'angle': 0,
          'props': {'grad': 'red', 'wave_strength': rand() * 2.}
        });
        //                this.subobj.push({'id': 'test_line2', 'x':  0, 'y': 0, 'x2':  0, 'y2': 0, 'props': { 'grad':
        //                'green', 'wave_strength': rand() * 20. } }); this.subobj.push({'id': 'test_line2', 'x': +s,
        //                'y': 0, 'x2': +s, 'y2': 0, 'props': { 'grad': 'blue', 'wave_strength': rand() * 20. } });
        /*
        this.subobj.push({'id': 'test_line2', 'x': this.x, 'y': this.y, 'x2': this.x2, 'y2': this.y2, 'props': { 'grad':
        'green' } });
        */
        /*
        this.subobj.push({'id': 'test_line2', 'x': this.x + 10, 'y': this.y, 'x2': this.x2 + 10, 'y2': this.y2,
        'props': { 'grad': 'blue' } });
    */
      },
      'time': function(time, elapsed) {
        this.angle += elapsed * 10.;
      },
    },
    'test_line2': {
      'type': 'line',
      'gradient': 'mix',
      'x': 0,
      'y': 0,
      'x2': 0,
      'y2': 0,
      'radius': 0.,
      'radiussize': 10.0,
      'opacity': 1.,
      'blending_type': blending_type.add,
      'angle': 0,
      'props': {},
      'scale': 1.0,
      'init': function() {
        // this.gradient = this.props.grad;
        this.props.x = this.x;
        this.props.y = this.y;
        this.props.angle = this.angle;
      },
      'time': function(time, elapsed) {
        var total_frames = script.video.duration * 25.;
        var current_frame = total_frames * time;
        // this.x =  this.props.x + triangular_wave(current_frame, 1., 1.) * this.props.wave_strength
        // this.y =  this.props.y + triangular_wave(current_frame, 1., 1.) * this.props.wave_strength
        // this.angle = this.props.angle + triangular_wave(current_frame, 1., 1.) * this.props.wave_strength
        //        this.angle += elapsed * 10.;
      },
    },
  },
  'video': {
    'duration': 5,
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 1.,
    'granularity': 1,
    'grain_for_opacity': true,
    'motion_blur': true,
    // 'min_intermediates': 10.,
    'max_intermediates': 10.,
    'update_positions': true,
  },
  'preview': {
    'width': 1920 / 5.,
    'height': 1920 / 5.,
    'max_intermediates': 1.,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      //{ 'id': 'test_line', 'x': 0., 'y': 0, 'x2': 0., 'y2': 0, 'props': {} },
      {
        'id': 'mother',
        'x': 0.,
        'y': 0,
        'props': {
          'scale': false,
        }
      },
      /*{
        'id': 'mother',
        'x': 0.,
        'y': 0,
        'angle': 45 / 2.,
        'props': {
          'scale': true,
        }
      },
       */
    ],
  }]
};
