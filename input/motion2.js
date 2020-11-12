_ = {
  'gradients': {
    'white': [
      {'position': 0.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1.0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
    ]
  },
  'objects': {
    'balls': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 10.0,
      'init': function() {
        class vector2d {
          constructor(x = 0, y = 0) {
            this.x = x;
            this.y = y;
          }
          rotate(degrees) {
            const radian = this.degrees_to_radian(degrees);
            const sine = Math.sin(radian);
            const cosine = Math.cos(radian);
            this.x = this.x * cosine - this.y * sine;
            this.y = this.x * sine + this.y * cosine;
          }
          degrees_to_radian(degrees) {
            const pi = 3.14159265358979323846;
            return degrees * pi / 180.0;
          }
        }

        // todo create width() and height() functions
        const width = 1920;
        const height = 1080;
        for (var i = 0; i < 300; i++) {
          let x = (rand() - 0.5) * 2 * width / 2;
          let y = (rand() - 0.5) * 2 * height / 2;
          let velocity = new vector2d(rand(), 0);
          velocity.rotate(rand() * 360);
          // velocity.x *= 10;
          // velocity.y *= 10;
          this.subobj.push(
              {'id': 'ball', 'x': x, 'y': y, 'z': 0, 'vel_x': velocity.x, 'vel_y': velocity.y, 'props': {}});
          velocity.rotate(rand() * 360);
        }
      },
      'time': function(t, e) {},
    },
    'ball': {
      'type': 'circle',
      'gradient': 'white',
      'radius': 0,
      'radiussize': 10.0,
      'props': {},
      'init': function() {},
      'time': function(t, elapsed) {},
    },
  },
  'video': {
    'duration': 10,
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 5,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      {'id': 'balls', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
