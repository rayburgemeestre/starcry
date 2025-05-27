/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface shape {
  x: number;
  y: number;
}

interface boundary {
  x: number;
  y: number;
  width: number;
  height: number;
  contains(x: number, y: number): boolean;
  intersects?(range: boundary): boolean;
}

export class rectangle implements boundary {
  x: number;
  y: number;
  width: number;
  height: number;

  constructor(x: number, y: number, width: number, height: number) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
  }

  contains(x: number, y: number): boolean {
    return x >= this.x && x < this.x + this.width && y >= this.y && y < this.y + this.height;
  }

  intersects(range: boundary): boolean {
    return !(
      range.x > this.x + this.width ||
      range.x + range.width < this.x ||
      range.y > this.y + this.height ||
      range.y + range.height < this.y
    );
  }
}

export class circle implements boundary {
  x: number;
  y: number;
  radius: number;
  width: number;
  height: number;

  constructor(x: number, y: number, radius: number) {
    this.x = x;
    this.y = y;
    this.radius = radius;
    // For Boundary interface compatibility
    this.width = radius * 2;
    this.height = radius * 2;
  }

  contains(x: number, y: number): boolean {
    const d = Math.pow(x - this.x, 2.0) + Math.pow(y - this.y, 2.0);
    return d <= Math.pow(this.radius, 2.0);
  }

  intersects(range: boundary): boolean {
    const xDist = Math.abs(range.x - this.x);
    const yDist = Math.abs(range.y - this.y);

    const r = this.radius;
    const w = range.width;
    const h = range.height;

    const edges = Math.pow(xDist - w, 2) + Math.pow(yDist - h, 2);

    // no intersection
    if (xDist > r + w || yDist > r + h) return false;

    // intersection within the circle
    if (xDist <= w || yDist <= h) return true;

    // intersection on the edge of the circle
    return edges <= Math.pow(this.radius, 2);
  }
}

export class point {
  shape: shape;
  userdata: number;

  constructor(shape: shape, userdata: number) {
    this.shape = shape;
    this.userdata = userdata;
  }
}

export class quadtree {
  boundary: rectangle;
  capacity: number;
  points: point[];
  divided: boolean;
  northwest?: quadtree;
  northeast?: quadtree;
  southeast?: quadtree;
  southwest?: quadtree;

  constructor(boundary: rectangle, capacity: number) {
    if (!boundary) {
      throw new TypeError('boundary is null or undefined');
    }
    if (!(boundary instanceof rectangle)) {
      throw new TypeError('boundary should be a Rectangle');
    }
    if (typeof capacity !== 'number') {
      throw new TypeError(`capacity should be a number but is a ${typeof capacity}`);
    }
    if (capacity < 1) {
      throw new RangeError('capacity must be greater than 0');
    }
    this.boundary = boundary;
    this.capacity = capacity;
    this.points = [];
    this.divided = false;
  }

  subdivide(): void {
    const x = this.boundary.x;
    const y = this.boundary.y;
    const w = this.boundary.width / 2;
    const h = this.boundary.height / 2;

    const nw = new rectangle(x + w, y, w, h);
    const ne = new rectangle(x + w, y + h, w, h);
    const se = new rectangle(x, y + h, w, h);
    const sw = new rectangle(x, y, w, h);

    this.northwest = new quadtree(nw, this.capacity);
    this.northeast = new quadtree(ne, this.capacity);
    this.southeast = new quadtree(se, this.capacity);
    this.southwest = new quadtree(sw, this.capacity);

    this.divided = true;
  }

  insert(point: point): boolean {
    if (!this.boundary.contains(point.shape.x, point.shape.y)) {
      return false;
    }
    if (this.points.length < this.capacity) {
      this.points.push(point);
      return true;
    }
    if (!this.divided) {
      this.subdivide();
    }
    return ((this.northeast && this.northeast.insert(point)) ||
      (this.northwest && this.northwest.insert(point)) ||
      (this.southeast && this.southeast.insert(point)) ||
      (this.southwest && this.southwest.insert(point))) as boolean;
  }

  query(range: boundary, found?: point[]): point[] {
    if (!found) {
      found = [];
    }
    if (!range.intersects || !range.intersects(this.boundary)) {
      return found;
    }
    for (const p of this.points) {
      if (range.contains(p.shape.x, p.shape.y)) {
        found.push(p);
      }
    }
    if (this.divided) {
      this.northwest?.query(range, found);
      this.northeast?.query(range, found);
      this.southwest?.query(range, found);
      this.southeast?.query(range, found);
    }
    return found;
  }
}
