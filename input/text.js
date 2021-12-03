_ = {
  'gradients': {
    'full_white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
    ],
    'white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 0.5, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ],
  },
  'objects': {
    'main': {
      'init': function() {},
      'time': function(t, e, scene, tt) {
        script.video.scale = (expf(tt, 20) * 3.0) + 1.0;
      },
    },
    'text_fixed': {
      'type': 'text',
      'gradient': 'full_white',
      'text': '',
      'text_size': 60,
      'text_align': 'center',
      'text_fixed': true,
      'props': {},
      'init': function() {
        this.props.x = this.x;
        this.props.y = this.y;
      },
      'time': function(t, e, s, tt) {
        // Try to keep it in frame..
        this.y = this.props.y / (4. + script.video.scale);
      },
    },
    'text_not_fixed': {
      'type': 'text',
      'gradient': 'full_white',
      'x': 0,
      'y': 0,
      'text': '',
      'text_size': 10,
      'text_align': 'center',
      'text_fixed': false,
      'props': {},
      'init': function() {
        this.props.y = this.y;
      },
      'time': function(t, e, s) {
        this.y = this.props.y / (4. + script.video.scale);
      },
    },
    'text_per_scene': {
      'type': 'text',
      'gradient': 'full_white',
      'x': 0,
      'y': 0,
      'text_size': 60,
      'text_align': 'center',
      'text_fixed': true,
      'init': function() {},
      'time': function(t, e, s) {
        this.text = script.scenes[s].name;
        this.text_size = t * script.scenes[s].max_text_size;
      },
    },
  },
  'video': {
    'fps': 30,
    'width': 1080,
    'height': 1080,
    'scale': 1,
    'rand_seed': 5,
    'granularity': 1,
    'bg_color': {'r': 0., 'g': 0., 'b': 0.5, 'a': 0},
  },
  'scenes': [
    {
      'name': 'This',
      'max_text_size': 200,
      'duration': 1,
      'objects': [
        {'id': 'main'},
        {'id': 'text_fixed', 'text': 'This is scale independent text', 'x': 0, 'y': 400, 'z': 0, 'props': {}},
        {'id': 'text_not_fixed', 'text': 'This is scale dependent text', 'x': 0, 'y': -400, 'z': 0, 'props': {}},
        {'id': 'text_per_scene', 'x': 0, 'y': 0, 'z': 0, 'props': {}},
      ]
    },
    {'name': 'is', 'max_text_size': 200, 'duration': 1, 'objects': []},
    {'name': 'Awesome!', 'max_text_size': 200, 'duration': 1, 'objects': []},
    {'name': '', 'duration': 1, 'objects': []},
  ]
};
