_ = {
  'gradients': {
    '5549ff54-aa4b-42a8-b3e7-3d5493c30fd0': [
      {'position': 0, 'r': 0.4392156862745098, 'g': 0.2549019607843137, 'b': 0.396078431372549, 'a': 0},
      {'position': 0.046, 'r': 0.4, 'g': 0.09411764705882353, 'b': 0.34509803921568627, 'a': 0},
      {'position': 0.095, 'r': 0.3764705882352941, 'g': 0.054901960784313725, 'b': 0.2980392156862745, 'a': 0},
      {'position': 0.103, 'r': 0.35294117647058826, 'g': 0.1411764705882353, 'b': 0.1568627450980392, 'a': 0},
      {'position': 0.114, 'r': 0.35294117647058826, 'g': 0.19607843137254902, 'b': 0.1411764705882353, 'a': 0},
      {'position': 0.125, 'r': 0.3607843137254902, 'g': 0.3254901960784314, 'b': 0.10588235294117647, 'a': 0},
      {'position': 0.136, 'r': 0.3607843137254902, 'g': 0.35294117647058826, 'b': 0.12941176470588237, 'a': 0},
      {'position': 0.144, 'r': 0.3607843137254902, 'g': 0.403921568627451, 'b': 0.1803921568627451, 'a': 0},
      {'position': 0.23, 'r': 0.34509803921568627, 'g': 0.5019607843137255, 'b': 0.09803921568627451, 'a': 0},
      {'position': 0.249, 'r': 0.33725490196078434, 'g': 0.5176470588235295, 'b': 0.09803921568627451, 'a': 0},
      {'position': 0.275, 'r': 0.33725490196078434, 'g': 0.5176470588235295, 'b': 0.0784313725490196, 'a': 0},
      {'position': 0.294, 'r': 0.33725490196078434, 'g': 0.49411764705882355, 'b': 0.16862745098039217, 'a': 0},
      {'position': 0.305, 'r': 0.3411764705882353, 'g': 0.4745098039215686, 'b': 0.2235294117647059, 'a': 0},
      {'position': 0.335, 'r': 0.36470588235294116, 'g': 0.5294117647058824, 'b': 0.19607843137254902, 'a': 0},
      {'position': 0.346, 'r': 0.37254901960784315, 'g': 0.5882352941176471, 'b': 0.5529411764705883, 'a': 0},
      {'position': 0.384, 'r': 0.403921568627451, 'g': 0.6039215686274509, 'b': 0.1450980392156863, 'a': 0},
      {'position': 0.395, 'r': 0.396078431372549, 'g': 0.49019607843137253, 'b': 0.11764705882352941, 'a': 0},
      {'position': 0.4039, 'r': 0.36470588235294116, 'g': 0.4470588235294118, 'b': 0.0784313725490196, 'a': 0},
      {'position': 0.41, 'r': 0.37254901960784315, 'g': 0.4980392156862745, 'b': 0.09803921568627451, 'a': 0},
      {'position': 0.414, 'r': 0.3686274509803922, 'g': 0.4980392156862745, 'b': 0.09411764705882353, 'a': 0},
      {'position': 0.436, 'r': 0.4196078431372549, 'g': 0.6509803921568628, 'b': 0.1450980392156863, 'a': 0},
      {'position': 0.444, 'r': 0.49019607843137253, 'g': 0.7686274509803922, 'b': 0.18823529411764706, 'a': 0},
      {'position': 0.673, 'r': 0.5529411764705883, 'g': 0.7176470588235294, 'b': 0.17254901960784313, 'a': 0},
      {'position': 0.688, 'r': 0.5450980392156862, 'g': 0.7058823529411765, 'b': 0.16470588235294117, 'a': 0},
      {'position': 0.924, 'r': 0.5098039215686274, 'g': 0.6470588235294118, 'b': 0.1568627450980392, 'a': 0},
      {'position': 0.976, 'r': 0.44313725490196076, 'g': 0.5529411764705883, 'b': 0.13725490196078433, 'a': 0},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
    ],
    'ce475a6c-2427-420c-85de-6316f3027313': [
      {'position': 0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'cd35b59b-bae9-4bef-b689-dcc8720e6be9': [
      {'position': 0, 'r': 0, 'g': 1, 'b': 0, 'a': 1},
      {'position': 1, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    '64f1da9f-07b2-46bd-8f62-30f9534f0cfd': [
      {'position': 0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ]
  },
  'objects': {
    'obj0': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 5.0,
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
        let velocity = new vector2d(rand(), 0);
        velocity.rotate(rand() * 360);
        this.subobj.push(
            {'id': 'obj1', 'x': -300, 'y': 0, 'z': 0, 'vel_x': velocity.x, 'vel_y': velocity.y, 'props': {}});
        velocity.rotate(rand() * 360);
        this.subobj.push(
            {'id': 'obj1', 'x': 300, 'y': 0, 'z': 0, 'vel_x': velocity.x, 'vel_y': velocity.y, 'props': {}});
        velocity.rotate(rand() * 360);
        this.subobj.push(
            {'id': 'obj1', 'x': 0, 'y': -300, 'z': 0, 'vel_x': velocity.x, 'vel_y': velocity.y, 'props': {}});
        velocity.rotate(rand() * 360);
        this.subobj.push(
            {'id': 'obj1', 'x': 0, 'y': +300, 'z': 0, 'vel_x': velocity.x, 'vel_y': velocity.y, 'props': {}});
      },
      'time': function(t, e) {},
    },
    'obj1': {
      'x': 0,
      'y': 0,
      'props': {
        'maxradius': 250.0,
      },
      'gradients': [
        [1.0, 'ce475a6c-2427-420c-85de-6316f3027313'],
        [0.0, 'cd35b59b-bae9-4bef-b689-dcc8720e6be9'],
        [0.0, '64f1da9f-07b2-46bd-8f62-30f9534f0cfd'],
      ],
      'subobj': [],
      'radius': 0,
      'radiussize': 5.0,
      'init': function() {
        this.subobj.push({'id': 'obj2', 'x': -100, 'y': 0, 'z': 0, 'props': {'maxradius': 100}});
        this.subobj.push({'id': 'obj2', 'x': 0, 'y': 0, 'z': 0, 'props': {'maxradius': 200}});
        this.subobj.push({'id': 'obj2', 'x': 100, 'y': 0, 'z': 0, 'props': {'maxradius': 300}});
      },
      'time': function(t, e) {
        this.radius = t * this.props.maxradius;
        this.gradients[0][0] = 1.0 - t;
        this.gradients[2][0] = t;
      },
      'proximity': function(t) {}
    },
    'obj2': {
      'type': 'circle',
      // 'gradient': 'ce475a6c-2427-420c-85de-6316f3027313',
      'radius': 100,
      'radiussize': 10.0,
      'props': {
        'maxradius': 250.0,
      },
      'init': function() {
        this.radius = 0;
      },
      'time': function(t, elapsed) {
        this.radius += 100.0 * elapsed;  // 25 pixels per second
        this.radius %= this.props.maxradius;
      },
      'proximity': function(t) {
        this.radiussize = 5.0 + (5.0 * t);
      }
    },
  },
  'video': {
    'duration': 10,
    'fps': 60,
    'width': 1920,
    'height': 1080,
    'scale': 1,
    'rand_seed': 5,
  },
  'scenes': [{
    'name': 'scene1',
    'objects': [
      // similar to a single "obj1"
      // {'id': 'obj2', 'x': -100, 'y': 0, 'z': 0, 'props': {'maxradius': 100}},
      // {'id': 'obj2', 'x': 0, 'y': 0, 'z': 0, 'props': {'maxradius': 200}},
      // {'id': 'obj2', 'x': 100, 'y': 0, 'z': 0, 'props': {'maxradius': 300}},

      // multiple instances of "obj1"s
      // {'id': 'obj1', 'x': -300, 'y': 0, 'z': 0, 'props': {}},
      // {'id': 'obj1', 'x': 300, 'y': 0, 'z': 0, 'props': {}},
      // {'id': 'obj1', 'x': 0, 'y': -300, 'z': 0, 'props': {}},
      // {'id': 'obj1', 'x': 0, 'y': +300, 'z': 0, 'props': {}},

      {'id': 'obj0', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
