script = {
  gradients: {
    white: [
      {position: 0, r: 1, g: 1, b: 1, a: 1},
      {position: 0.15, r: 1, g: 1, b: 1, a: 1},
      {position: 1, r: 1, g: 1, b: 1, a: 0},
    ],
  },
  textures: {
    clouds1: {
      type: 'perlin',
      size: 3000,
      octaves: 7,
      persistence: 0.45,
      percentage: 1,
      scale: 2000,
      range: [0, 0, 0.2, 0.8],
      strength: 1,
      speed: 1,
    },
    zernike1: {
      type: 'zernike',
      n: 7,
      m: -3,
      zernike_type: zernike_type.version1,
      effect: texture_effect.opacity,
    },
  },
  objects: {
    main: {
      init: function() {
        // Spawn multiple objects to demonstrate emit/absorb effects
        this.spawn({id: 'emitter1', x: -200, y: 0});
        this.spawn({id: 'emitter2', x: 200, y: 0});
        this.spawn({id: 'absorber1', x: 0, y: -100});
        this.spawn({id: 'absorber2', x: 0, y: 100});
      },
      time: function(t, e, s, T) {
        // Main controller object
      },
    },

    // Object that emits proximity and age effects
    emitter1: {
      x: 0,
      y: 0,
      radius: 0,
      radiussize: 30,
      init: function() {
        // this.start_time = e; // Store creation time
      },
      time: function(t, e, scene) {
        // Move in a circle
        this.x = Math.cos(t * Math.PI * 2) * 150;
        this.y = Math.sin(t * Math.PI * 2) * 150;
      },
      emit: {
        prox: {
          type: 'proximity',
          max: 200,  // Maximum distance in pixels
          range_start: 0.0,
          range_end: 1.0,
        },
        age: {
          type: 'age',
          max: 5,  // Maximum age in seconds
          range_start: 0.0,
          range_end: 1.0,
        },
      },
      gradient: 'white',
      texture: 'clouds1',
    },

    // Another emitter with different settings
    emitter2: {
      x: 0,
      y: 0,
      radius: 0,
      radiussize: 40,
      init: function() {
        this.mass = 2.0;  // Higher mass
      },
      time: function(t, e, scene) {
        // Move in opposite direction
        this.x = Math.cos(-t * Math.PI * 2) * 100;
        this.y = Math.sin(-t * Math.PI * 2) * 100;
      },
      emit: {
        prox: {
          type: 'proximity',
          max: 150,
          range_start: 0.0,
          range_end: 0.5,  // Weaker proximity effect
        },
        gravity: {
          type: 'mass',
          max: 300,  // Affects objects within 300 pixels
          range_start: 0.0,
          range_end: 1.0,
        },
      },
      gradient: 'white',
      texture: 'zernike1',
      hue: 180,
    },

    // Object that absorbs proximity to control glow and age to fade in
    absorber1: {
      x: 0,
      y: 0,
      radius: 0,
      radiussize: 25,
      init: function() {
        this.glow = 0;
        this.opacity = 0;
      },
      time: function(t, e, scene) {
        // Stationary absorber
      },
      absorb: {
        glow_effect: {
          input: 'prox',
          range_start: 0.0,
          range_end: 1.0,
          effect: function(intensity) {
            this.glow = intensity * 0.8;            // Glow based on proximity
            this.radiussize = 25 + intensity * 15;  // Also grow
          },
        },
        fade_in: {
          input: 'age',
          range_start: 0.0,
          range_end: 1.0,
          effect: function(intensity) {
            this.opacity = intensity;  // Fade in over time
          },
        },
      },
      gradient: 'white',
      hue: 60,
    },

    // Object that absorbs mass effect to be attracted
    absorber2: {
      x: 0,
      y: 0,
      radius: 0,
      radiussize: 20,
      init: function() {
        this.velocity_x = 0;
        this.velocity_y = 0;
      },
      time: function(t, e, scene) {
        // Apply velocity
        this.x += this.velocity_x;
        this.y += this.velocity_y;

        // Damping
        this.velocity_x *= 0.95;
        this.velocity_y *= 0.95;

        // Keep within bounds
        if (Math.abs(this.x) > 400) this.velocity_x *= -1;
        if (Math.abs(this.y) > 300) this.velocity_y *= -1;
      },
      absorb: {
        attraction: {
          input: 'gravity',
          range_start: 0.0,
          range_end: 0.1,  // Scale down the effect
          effect: function(intensity, emitter) {
            // Calculate direction towards emitter
            if (emitter) {
              var dx = emitter.x - this.x;
              var dy = emitter.y - this.y;
              var dist = Math.sqrt(dx * dx + dy * dy);
              if (dist > 0) {
                // Apply force proportional to intensity
                this.velocity_x += (dx / dist) * intensity * 2;
                this.velocity_y += (dy / dist) * intensity * 2;
              }
            }
          },
        },
        color_shift: {
          input: 'prox',
          range_start: 0.0,
          range_end: 1.0,
          effect: function(intensity) {
            this.hue = 120 + intensity * 180;  // Shift from green to purple
            this.opacity = 0.5 + intensity * 0.5;
          },
        },
      },
      gradient: 'white',
      texture: 'clouds1',
    },

    // Text label for emitter1
    text1: {
      type: 'text',
      text: 'test',
      text_size: 60,
      text_align: 'center',
      text_fixed: true,
    },
  },
  video: {
    fps: 25,
    width: 1920,
    height: 1080,
    scale: 5,
    rand_seed: 5,
    granularity: 1,
    minimize_steps_per_object: false,
    bg_color: {r: 0, g: 0, b: 0, a: 1},
    grain_for_opacity: false,
    gamma: 1,
  },
  preview: {width: 512, height: 512},
  scenes: [
    {
      name: 'scene',
      duration: 10,
      objects: [{id: 'main', x: 0, y: 0, z: 0}],
    },
  ],
};
