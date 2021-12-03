_ = {
  'gradients': {
    'full_white': [
      {'position': 0, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
      {'position': 1, 'r': 1, 'g': 1, 'b': 1, 'a': 1},
    ],
  },
  'objects': {
    'text_fixed': {
      'type': 'text',
      'gradient': 'full_white',
      'text': '',
      'text_size': 60,
      'text_align': 'center',
      'text_fixed': true,
      'props': {},
      'init': function() {},
      'time': function(t, e, s, tt) {
        script.video.scale = (expf(tt, 10000) * 3.0) + 1.0;
        // DEBUG: hehe, this creates an infinite render loop
        this.y--;
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
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 0},
    'grain_for_opacity': true,
  },
  'scenes': [
    {
      'name': 'This',
      'max_text_size': 200,
      'duration': 1,
      'objects': [
        {'id': 'text_fixed', 'text': 'This is scale independent text', 'x': 0, 'y': 300, 'z': 0, 'props': {}},
      ]
    },
    {'name': 'is', 'max_text_size': 200, 'duration': 1, 'objects': []},
    {'name': 'Awesome!', 'max_text_size': 200, 'duration': 1, 'objects': []},
    {'name': '', 'duration': 1, 'objects': []},
  ]
};
