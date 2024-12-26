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
        const radius = 50;  // Minimum distance between points
        const points = generatePoissonPoints(canvas_size, radius);

        points.forEach(point => {
          this.spawn(
              {'id': 'point', 'x': point.x - canvas_size / 2, 'y': point.y - canvas_size / 2, 'z': 0, 'props': {}});
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

function generatePoissonPoints(width, radius) {
  const k = 30;  // Number of attempts before rejection
  const grid = [];
  const active = [];
  const points = [];
  const cellSize = radius / Math.sqrt(2);

  // Initialize grid
  const cols = Math.ceil(width / cellSize);
  const rows = Math.ceil(width / cellSize);
  for (let i = 0; i < cols * rows; i++) grid[i] = undefined;

  // Add first point
  const firstPoint = {x: width / 2, y: width / 2};
  points.push(firstPoint);
  active.push(firstPoint);
  grid[gridIndex(firstPoint, cols, cellSize)] = points.length - 1;

  // Try to generate points until no active points remain
  while (active.length > 0 && points.length < 200) {  // Limit to 200 points
    const randomIndex = Math.floor(rand() * active.length);
    const point = active[randomIndex];
    let found = false;

    // Try k times to find a valid new point
    for (let n = 0; n < k; n++) {
      const angle = rand() * Math.PI * 2;
      const distance = rand() * radius + radius;
      const sample = {x: point.x + Math.cos(angle) * distance, y: point.y + Math.sin(angle) * distance};

      if (isValidPoint(sample, width, width, radius, points, grid, cols, rows, cellSize)) {
        points.push(sample);
        active.push(sample);
        grid[gridIndex(sample, cols, cellSize)] = points.length - 1;
        found = true;
        break;
      }
    }

    // If no valid point was found after k attempts, remove the current point from active list
    if (!found) {
      active.splice(randomIndex, 1);
    }
  }

  return points;
}

function gridIndex(point, cols, cellSize) {
  const col = Math.floor(point.x / cellSize);
  const row = Math.floor(point.y / cellSize);
  return col + row * cols;
}

function isValidPoint(point, width, height, radius, points, grid, cols, rows, cellSize) {
  if (point.x < 0 || point.x >= width || point.y < 0 || point.y >= height) {
    return false;
  }

  const col = Math.floor(point.x / cellSize);
  const row = Math.floor(point.y / cellSize);

  // Check nearby cells
  const searchRadius = 2;
  for (let i = -searchRadius; i <= searchRadius; i++) {
    for (let j = -searchRadius; j <= searchRadius; j++) {
      const searchCol = col + i;
      const searchRow = row + j;
      if (searchCol >= 0 && searchCol < cols && searchRow >= 0 && searchRow < rows) {
        const pointIndex = grid[searchCol + searchRow * cols];
        if (pointIndex !== undefined) {
          const nearby = points[pointIndex];
          const dx = nearby.x - point.x;
          const dy = nearby.y - point.y;
          if (dx * dx + dy * dy < radius * radius) {
            return false;
          }
        }
      }
    }
  }
  return true;
}
