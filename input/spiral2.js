_ = {
  'gradients': {
    'green': [
      {'position': 0, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 0.9, 'r': 1, 'g': 0, 'b': 0, 'a': 1},
      {'position': 1, 'r': 1, 'g': 0, 'b': 0, 'a': 0},
    ]
  },
  'objects': {
    'spiral': {
      'init': function() {
        let spawn_pixel = (x, y) => {
          this.spawn({'id': 'point2', 'x': x, 'y': y, 'z': 0, 'props': {}});
        };
        const checkerboardSize = 500;
        const squareSize = 10;

        function mapCheckerboardToSphere(x, y, r) {
          const theta = 2 * Math.PI * x / (checkerboardSize - 1);
          const phi = Math.PI * y / (checkerboardSize - 1);

          const nx = Math.sin(phi) * Math.cos(theta);
          const ny = Math.sin(phi) * Math.sin(theta);
          const nz = Math.cos(phi);

          const px = r * nx;
          const py = r * ny;
          const pz = r * nz;

          return {px, py, pz};
        }
        function getPixel(x, y) {
          let xCell = Math.floor(x / squareSize);
          let yCell = Math.floor(y / squareSize);
          return ((xCell + yCell) % 2 === 0);
        }

        const radius = checkerboardSize * 0.5 - 10;
        const centerX = checkerboardSize * 0.5;
        const centerY = checkerboardSize * 0.5;
        const zOffset = 100;

        for (let y = 0; y < checkerboardSize; y += 5) {
          for (let x = 0; x < checkerboardSize; x += 5) {
            const mappedPoint = mapCheckerboardToSphere(x, y, radius);
            const z = mappedPoint.pz + zOffset;

            if (z > 0) {
              const pixel = getPixel(x, y);
              if (pixel) {
                const px = centerX + (mappedPoint.px / z) * zOffset;
                const py = centerY - (mappedPoint.py / z) * zOffset;
                spawn_pixel(px - checkerboardSize / 2, py - checkerboardSize / 2);
              }
            }
          }
        }
      },
      'time': function(t, e, scene) {
        // this.rotate = 360. * t;
      },
    },
    'point': {
      'type': 'circle',
      'radius': 0,
      'radiussize': 1.5,
      'gradient': 'green',
      'blending_type': blending_type.normal,
      'gravity_group': 'g1',
      'mass': 1.0,
      'velocity': 0.1,
      'time': function(t, e, scene) {
        // this.radiussize += e * 10;
        this.rotate += e * 10;
      },
    },
    'point2': {
      'type': 'script',
      'file': 'input/object2.js',
      'blending_type': blending_type.add,
      'gravity_group': 'g1',
      'opacity': 0.2,
      'mass': 1.0,
      'duration': 5.0,
      'velocity': 0.1,
      // 'radiussize': 10.0,
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1920,
    'scale': 6.0,
    'rand_seed': 5,
    'granularity': 1,
    'minimize_steps_per_object': false,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
    'grain_for_opacity': false,
    'min_intermediates': 10,
    'max_intermediates': 10,
    'gravity_range': 50,
    'gravity_G': 0.02,
    'gravity_constrain_dist_min': 5.,
    'gravity_constrain_dist_max': 5.,
    'perlin_noise': true,
  },
  'preview': {
    'width': 512,
    'height': 512,
  },
  'scenes': [
    {'name': 'scene1', 'duration': 30.0, 'objects': [{'id': 'spiral', 'x': 0, 'y': 0, 'z': 0, 'props': {}}]},
  ]
};
