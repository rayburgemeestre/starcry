_ = {
  'gradients': {
    'rainbow': [
      {'position': 0 / 7., 'r': 220 / 255., 'g': 29 / 255., 'b': 11 / 255., 'a': 1},
      {'position': 2 / 7., 'r': 251 / 255., 'g': 129 / 255., 'b': 0 / 255., 'a': 1},
      {'position': 3 / 7., 'r': 245 / 255., 'g': 225 / 255., 'b': 12 / 255., 'a': 1},
      {'position': 4 / 7., 'r': 71 / 255., 'g': 213 / 255., 'b': 53 / 255., 'a': 1},
      {'position': 5 / 7., 'r': 43 / 255., 'g': 15 / 255., 'b': 223 / 255., 'a': 1},
      {'position': 6 / 7., 'r': 194 / 255., 'g': 16 / 255., 'b': 169 / 255., 'a': 1},
      {'position': 7 / 7., 'r': 194 / 255., 'g': 16 / 255., 'b': 169 / 255., 'a': 0},
    ],
  },
  'objects': {
    'mother': {
      'init': function() {
        let types = [
          blending_type.normal,      blending_type.lighten,    blending_type.darken,      blending_type.multiply,
          blending_type.average,     blending_type.add,        blending_type.subtract,    blending_type.difference,
          blending_type.negation,    blending_type.screen,     blending_type.exclusion,   blending_type.overlay,
          blending_type.softlight,   blending_type.hardlight,  blending_type.colordodge,  blending_type.colorburn,
          blending_type.lineardodge, blending_type.linearburn, blending_type.linearlight, blending_type.vividlight,
          blending_type.pinlight,    blending_type.hardmix,    blending_type.reflect,     blending_type.glow,
          blending_type.phoenix,     blending_type.hue,        blending_type.saturation,  blending_type.color,
          blending_type.luminosity
        ];
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
        for (let type of types) {
          let velocity = new vector2d(rand(), 0);
          velocity.rotate(rand() * 360);
          this.subobj.push({
            'id': 'rainbow',
            'blending_type': type,
            'x': (rand() * 1920) - 1920 / 2.,
            'y': (rand() * 1080) - 1080 / 2.,
            'vel_x': velocity.x,
            'vel_y': velocity.y,
            'velocity': (rand() * 5.) + 5.,
            'z': 0,
            'props': {}
          });
        }
      },
      'time': function(t, e, scene) {},
    },
    'rainbow': {
      'type': 'circle',
      'gradient': 'rainbow',
      'x': 0,
      'y': 0,
      'radius': 0,
      'radiussize': 300.0,
      'angle': 0.,
      'init': function() {},
      'time': function(t, e, scene) {
        while (this.x + (1920 / 2) < 0) this.x += 1920;
        while (this.y + (1080 / 2) < 0) this.y += 1080;
        while (this.x + (1920 / 2) > 1920) this.x -= 1920;
        while (this.y + (1080 / 2) > 1080) this.y -= 1080;
      },
    },
  },
  'video': {
    'fps': 30,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 5,
    'granularity': 1,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 0},
    'grain_for_opacity': false,
  },
  'preview': {
    'max_intermediates': 1.,
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 30,
      'objects': [
        {'id': 'mother', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      ]
    },
  ]
};
