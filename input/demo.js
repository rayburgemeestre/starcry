{
  'gradients': {
    'color1': '#5800FF' +
        '@0.0',
    'color2': '#0096FF' +
        '@0.0',
    'color2b': '#0096FF' +
        '@0.9',
    'color3': '#00D7FF' +
        '@0.0',
    'color4': '#72FFFF' +
        '@0.0',
    'color5': '#333333' +
        '@0.0',
  },
  'objects': {
    'flower': {
      'init': function() {
        function radians(degrees) {
          return degrees * (Math.PI / 180);
        }

        let circles = [];
        let non_overlapping = [];
        let seen = new Set();
        circles.push([this.x, this.y]);
        // negative coordinates ??
        seen.add(parseInt(this.x) + ',' + parseInt(this.y));

        while (circles.length > 0) {
          // let [x, y] = circles.pop();
          // pop first element instead
          let [x, y] = circles.shift();

          let dist = Math.sqrt(x * x + y * y);
          function clamp(x, min, max) {
            return Math.max(min, Math.min(x, max));
          }
          let max_dist = 500;
          let overlapping = false;
          for (let other of non_overlapping) {
            let other_x = other[0];
            let other_y = other[1];

            // check if there is overlap between x, y and other_x, other_y
            // both have a radius of 100
            let circles_overlap = Math.sqrt((x - other_x) * (x - other_x) + (y - other_y) * (y - other_y)) < 190;
            if (circles_overlap) {
              overlapping = true;
              break;
            }
          }
          let opacity = 1. - clamp(dist / (max_dist), 0.0, 1.0);
          if (overlapping) {
            opacity /= 4;
          } else {
            opacity = 1. - clamp(dist / (max_dist), 0.0, 1.0);
            non_overlapping.push([x, y]);
          }
          this.spawn({
            'id': 'flowercircle',
            'x': x,
            'y': y,
            'opacity': opacity,
          });
          for (let n = 6, i = 0; i < n; i++) {
            let t = (360 / n) * i - 90;
            let newx = x + (100 * Math.cos(radians(t)));
            let newy = y + (100 * Math.sin(radians(t)));
            // calculate distance from 0,0
            let dist = Math.sqrt(newx * newx + newy * newy);
            if (dist < max_dist && !seen.has(parseInt(newx) + ',' + parseInt(newy))) {
              seen.add(parseInt(newx) + ',' + parseInt(newy));
              circles.push([newx, newy]);
            }
          }
        }
        this.spawn_parent({
          'id': 'edge',
          'opacity': 1,
          'radius': 500,
        });
      },
      'time': function(t, e, s, tt) {},
    },
    'flowercircle': {
      'type': 'circle',
      'unique_group': 'group1',
      'gradient': 'color1',
      'radius': 100,
      'radiussize': 2.5,
      'init': function() {},
      'time': function(t, e, s, tt) {},
    },
    'edge': {
      'type': 'circle',
      //'unique_group': 'group2',
      'gradient': 'color2',
      'radius': 150,
      'radiussize': 0.0,
      'init': function() {
        function radians(degrees) {
          return degrees * (Math.PI / 180);
        }
        let radius = this.radius;
        let circle_size = 5. * 2;
        let circumference = 2 * Math.PI * radius;
        // fill entire radius of the circle with small 33.3 sized circles
        let n = Math.floor(circumference / circle_size);
        for (let i = 0; i < n; i++) {
          let t = (360 / n) * i;
          let x = radius * Math.cos(radians(t));
          let y = radius * Math.sin(radians(t));
          this.spawn({
            'id': 'edgering',
            'x': x,
            'y': y,
            'opacity': 1,
          });
        }
      },
      'time': function(t, e, s, tt) {},
    },
    'edgering': {
      'type': 'circle',
      'gradient': 'color3',
      'radius': 4,
      'radiussize': 1.5,
      'init': function() {},
      'time': function(t, e, s, tt) {},
    },
    'layer4': {
      'type': 'circle',
      'gradient': 'color2',
      'radius': 200,
      'radiussize': 5.0,
      'init': function() {
        function radians(degrees) {
          return degrees * (Math.PI / 180);
        }
        for (let n = 6, i = 0; i < n; i++) {
          let t = (360 / n) * i - 90;
          let newx = this.x + (200 * Math.cos(radians(t)));
          let newy = this.y + (200 * Math.sin(radians(t)));
          this.spawn({
            'id': 'perlin',
            'x': newx,
            'y': newy,
            'opacity': 1,
            'attrs': {
              'radiussize': 50.0,
            }
          });
        }
      },
      'time': function(t, e, s, tt) {},
    },
    'layer5': {
      'type': 'circle',
      'gradient': 'color2b',
      'radius': 0,
      'radiussize': 50.0,
      'hue': 180,
      'init': function() {},
      'time': function(t, e, s, tt) {},
    },
    'layer6': {
      'type': 'ellipse',
      'shortest_diameter': 100,
      'longest_diameter': 500,
      'gradient': 'color2',
      'radius': 0,
      'radiussize': 2.5,
      'opacity': 0.5,
      'hue': 180,
      'blending_type': blending_type.phoenix,
      'init': function() {},
      'time': function(t, e, s, tt) {
        this.shortest_diameter = Math.max(200, Math.min(300, t * 1.1 * 500));
        // this.rotate = t * 360;
      },
    },
    'perlin': {
      'type': 'script',
      'file': 'input/perlin.js',
    },
  },
  'video': {
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'min_intermediates': 1,
    'max_intermediates': 1,
    'grain_for_opacity': false,
    'scale': 1.,
  },
  'preview': {
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {
      'name': 'scene1',
      'duration': 10.0,
      'objects': [
        {'id': 'flower', 'x': 0, 'y': 0, 'z': 0},
        // {'id': 'layer4', 'x': 0, 'y': 0, 'z': 0},
        // {'id': 'layer6', 'x': 0, 'y': 0, 'z': 0},
        // {'id': 'layer6', 'x': 0, 'y': 0, 'z': 0, 'rotate': 45},
        // {'id': 'layer6', 'x': 0, 'y': 0, 'z': 0, 'rotate': 90},
        // {'id': 'layer6', 'x': 0, 'y': 0, 'z': 0, 'rotate': -45},
      ]
    },
  ]
};
