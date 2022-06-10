_ = {
  'gradients': {
    'red': [
      {'position': 0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
    'green': [
      {'position': 0, 'r': 0, 'g': 1, 'b': 0, 'a': 1},
      {'position': 1, 'r': 0, 'g': 1, 'b': 0, 'a': 0},
    ],
    'blue': [
      {'position': 0, 'r': 0, 'g': 0, 'b': 1, 'a': 1},
      {'position': 1, 'r': 0, 'g': 0, 'b': 1, 'a': 0},
    ]
  },
  'objects': {
    'top-level-object': {
      'props': {},
      'subobj': [],
      'init': function() {
        // place four circle animations on the canvas
        this.subobj.push(this.spawn({'id': 'simple-animation-object', 'x': -300, 'y': 0, 'z': 0, 'props': {}}));
        this.subobj.push(this.spawn({'id': 'simple-animation-object', 'x': 300, 'y': 0, 'z': 0, 'props': {}}));
        this.subobj.push(this.spawn({'id': 'simple-animation-object', 'x': 0, 'y': -300, 'z': 0, 'props': {}}));
        this.subobj.push(this.spawn({'id': 'simple-animation-object', 'x': 0, 'y': +300, 'z': 0, 'props': {}}));
      },
      'time': function(t) {},
    },
    'simple-animation-object': {
      'gradients': [
        [1.0, 'red'],
        [0.0, 'green'],
        [0.0, 'blue'],
      ],
      'subobj': [],
      'init': function() {
        // place three animated circles with different properties
        this.subobj.push(this.spawn({'id': 'animated-circle', 'x': -100, 'y': 0, 'z': 0, 'props': {'maxradius': 100}}));
        this.subobj.push(this.spawn({'id': 'animated-circle', 'x': 0, 'y': 0, 'z': 0, 'props': {'maxradius': 200}}));
        this.subobj.push(this.spawn({'id': 'animated-circle', 'x': 100, 'y': 0, 'z': 0, 'props': {'maxradius': 300}}));
      },
      'time': function(t) {
        // tween the colors of all animated circles
        this.gradients[0][0] = 1.0 - t;
        this.gradients[2][0] = t;
      },
    },
    'animated-circle': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 5.0,
      'props': {
        'maxradius': 250.0,
      },
      'init': function() {},
      'time': function(t) {
        this.radius += 5;
        this.radius %= this.props.maxradius;
      },
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
  },
  'scenes': [{
    'name': 'scene1',
    'duration': 10,
    'objects': [
      {'id': 'top-level-object', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
    ],
  }]
};
