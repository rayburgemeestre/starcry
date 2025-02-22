script = {
  'gradients': {'white': '#ffffff@0.8'},
  'objects': {
    'main': {
      'x': 0,
      'y': 0,
      'props': {},
      'subobj': [],
      'radius': 0,
      'radiussize': 0.0,
      'angle': 0.,
      'init': function() {
        const canvas_size = 2000;
        const radius = 50;
        const points = generatePoints(canvas_size, radius);

        points.forEach(point => {
          let [vel_x, vel_y] = random_velocity();
          this.spawn({
            'id': 'point',
            'x': point.x - canvas_size / 2,
            'y': point.y - canvas_size / 2,
            'z': 0,
            'vel_x': vel_x,
            'vel_y': vel_y,
            'velocity': 1,
            'props': {}
          });
        });
      },
      'time': function(t, e, scene) {},
    },
    'point': {
      'x': 0,
      'y': 0,
      'type': 'circle',
      'gradient': 'white',
      'radius': 0,
      'radiussize': 10,
      'init': function() {},
    },
  },
  'video': {
    'fps': 25,
    'width': 1920,
    'height': 1080,
    'bg_color': {'r': 0., 'g': 0., 'b': 0., 'a': 1},
  },
  'scenes': [
    {'name': 'scene1', 'duration': 60.0, 'objects': [{'id': 'main', 'x': 5, 'y': 0, 'z': 0, 'props': {}}]},
  ]
};

function generatePoints(width, radius) {
  const points = [];
  const candidates = 10;  // Number of candidates to generate for each point

  // Add first point in the center
  points.push({x: width / 2, y: width / 2});

  // Generate remaining points
  while (points.length < 200) {
    let bestCandidate = null;
    let bestDistance = -1;

    // Generate and test several candidates
    for (let i = 0; i < candidates; i++) {
      const candidate = {x: rand() * width, y: rand() * width};

      // Find distance to closest existing point
      let minDistance = width * width;  // Square of max possible distance
      for (const point of points) {
        const dx = candidate.x - point.x;
        const dy = candidate.y - point.y;
        const distance = dx * dx + dy * dy;
        if (distance < minDistance) {
          minDistance = distance;
        }
      }

      // Update best candidate if this one is better
      if (minDistance > bestDistance) {
        bestDistance = minDistance;
        bestCandidate = candidate;
      }
    }

    // Add the best candidate to our points
    points.push(bestCandidate);
  }

  return points;
}
