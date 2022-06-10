_ = {
  'gradients': {
    'blue': [
      // 64, 179, 159
      //{'position': 0.0, 'r': 64/255., 'g': 179/255., 'b': 159/255., 'a': 1.0},
      //{'position': 0.0, 'r': 0.5, 'g': 0.5, 'b': 0.5, 'a': 0.5},
      {'position': 0.0, 'r': 0., 'g': 0., 'b': 1., 'a': 0.1},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
    'white_debug': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1.0},
    ],
    'white_line': [
      {'position': 0.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
      {'position': 0.5, 'r': 1, 'g': 1, 'b': 1, 'a': 1.0},
      {'position': 1.0, 'r': 0, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'textures': {
    'clouds': {
      'type': 'perlin',  // fractal, turbulence
      'size': 1920.,
      'octaves': 7,
      'persistence': 0.45,
      'percentage': 1.0,
      'scale': 14.,
      'range': [0.0, 0.0, 1.0, 1.0],
      'strength': 0.8,
    }
  },
  'objects': {
    'line': {
      'type': 'line',
      'gradient': 'white',
      // 'texture': 'clouds',
      'radius': 0,
      'x': 0,
      'y': 0,
      'x2': 0,
      'y2': 0,
      'radiussize': 10.0,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {},
    },
    'random_lines': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'init': function() {
        var prev_x = 0;
        var prev_y = 0;
        var width = script.video.width;
        var height = script.video.height;
        var prev_vel_x = 0;
        var prev_vel_y = 0;

        var vel_x = 0;
        var vel_y = 0;

        for (var i = 0; i < 50; i++) {
          var x = (rand() * width) - width / 2;
          var y = (rand() * height) - height / 2;
          [vel_x, vel_y] = random_velocity();
          if (i > 0) {
            this.subobj.push(this.spawn({
              'id': 'line',
              'x': x,
              'y': y,
              'x2': prev_x,
              'y2': prev_y,
              'vel_x': vel_x,
              'vel_y': vel_y,
              'vel_x2': prev_vel_x,
              'vel_y2': prev_vel_y,
              'velocity': 300,
              'z': 0,
              'radiussize': 5.0,
              'props': {}
            }));
          }
          prev_x = x;
          prev_y = y;
          prev_vel_x = vel_x;
          prev_vel_y = vel_y;
        }
      },
      'time': function(t, elapsed) {
        script.video.scale = 1.;
        script.video.scale += 9. * t;
      },
    },
    'bg': {
      'type': 'circle',
      'gradient': 'blue',
      'radius': 0,
      'radiussize': 0,
      'init': function() {
        this.radiussize = script.video.width;
      },
      'time': function(t, elapsed) {},
    },
  },
  'video': {
    'fps': 30,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 1,
    'granularity': 1,
    'update_positions': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 60,
    'objects': [
      {'id': 'bg', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      {'id': 'random_lines', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
