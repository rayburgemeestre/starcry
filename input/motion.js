/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
include('lib/vectors.js')

const fps = 25;
const max_frames = 250;
const canvas_w = 1920;
const canvas_h = 1080;
const scale = 1;

const resolution = 0.5;
const speed = 1;
const num_balls = 3000;
const ball_radius = 5.;

let balls = [];

class projection
{
    constructor(canvas_w, canvas_h) {
        this.w = canvas_w;
        this.h = canvas_h;
        this.half_w = canvas_w / 2;
        this.half_h = canvas_h / 2;
    }
    random_position() {
        return this.translate(new vector2d(
            Math.random() * this.w,
            Math.random() * this.h
        ));
    }
    translate(position) {
        // translate from centered (0,0 == middle of canvas) to top left oriented (0,0 == top left).
        return new vector2d(
            position.x - this.half_w,
            position.y - this.half_h
        );
    }
    wrap_position(position)
    {
        while (position.x < -this.half_w) position.x += this.w;
        while (position.y < -this.half_h) position.y += this.h;
        while (position.x > this.half_w) position.x -= this.w;
        while (position.y > this.half_h) position.y -= this.h;
        return new vector2d(position.x, position.y);
    }
}

class simple_shape
{
    constructor()
    {
        this.velocity = new vector2d(1, 0);
        this.velocity.x = Math.random();
        this.velocity.rotate(Math.random() * 360);

        this.gradient_ = new gradient();
        this.gradient_.add(0.0, new color(1, 1, 1, 1))
        this.gradient_.add(0.9, new color(1, 1, 1, 1));
        this.gradient_.add(1.0, new color(0, 1, 0, 0));
    }
}

// testing..
class local_circle
{
    constructor(pos, radius, radius_size, gradient, bt)
    {
        this.blending_type = bt ? bt : blending_type.normal;
        this.pos = pos;
        this.pos_vec = new vector2d(pos.x, pos.y);
        this.radius = radius;
        this.radius_size = radius_size;
        this.gradient = gradient;
        this.blending_type = blending_type;
    }
    as_vec2d() {
        this.pos_vec.x = this.pos.x;
        this.pos_vec.y = this.pos.y;
        return this.pos_vec;
    }
}

class ball extends simple_shape
{
    constructor(view) {
        super();
        this.view = view;
        this.circle_ = new circle(new pos(0, 0, 0), 0, ball_radius, this.gradient_);
        this.circle_.blending_type = blending_type.add;
        const rand = this.view.random_position();
        this.circle_.x = rand.x;
        this.circle_.y = rand.y;
    }
    move() {
        this.circle_.x += this.velocity.x / resolution;
        this.circle_.y += this.velocity.y / resolution;
        let new_vec = this.view.wrap_position(this.circle_.as_vec2d());
        this.circle_.x = new_vec.x;
        this.circle_.y = new_vec.y;
    }
    stop() {
        this.velocity.x = 0;
        this.velocity.y = 0;
    }
    draw() {
        add_circle(this.circle_);
    }
}

function initialize()
{
    let view = new projection(canvas_w, canvas_h);
    for (let i=0; i<num_balls; i++)
        balls.push(new ball(view));
}

function circles_collide(a, b)
{
    const radius_sum = a.radius_size + b.radius_size;
    const a_center = a.as_vec2d();
    const b_center = b.as_vec2d();
    const distance = subtract_vector(a_center, b_center);
    return dot_product(distance, distance) <= radius_sum * radius_sum;
}

function already_collided(a, b)
{
    return (a.last_collide && a.last_collide == b && b.last_collide && b.last_collide == a);
}

function collide_balls()
{
    for (let i=0; i<balls.length; i++) {
        for (let j=i+1; j<balls.length; j++) {
            if (i == j) continue;
            if (!circles_collide(balls[i].circle_, balls[j].circle_) || already_collided(balls[i], balls[j]))
                continue;
            const a_center = balls[i].circle_.as_vec2d();
            const b_center = balls[j].circle_.as_vec2d();
            // excerpt from http://www.gamasutrballs[i].com/view/feature/3015/pool_hall_lessons_fast_accurate_.php?page=3
            // got mine from http://wonderfl.net/c/rp7P
            var normal = unit_vector(subtract_vector(a_center, b_center));
            var ta = dot_product(balls[i].velocity, normal);
            var tb = dot_product(balls[j].velocity, normal);
            var optimized_p = (2.0 * (ta - tb)) / 2.0;
            balls[i].velocity = subtract_vector(balls[i].velocity, multiply_vector(normal, optimized_p));
            balls[j].velocity = add_vector(balls[j].velocity, multiply_vector(normal, optimized_p));
            balls[i].last_collide = balls[j];
            balls[j].last_collide = balls[i];
        }
    }
}

function next()
{
    set_background_color(new color(0, 0, 0, 255))
        for (let d of balls) {
            d.move();;
            for (let i=0; i<speed; i++) {
                collide_balls();
        }
    }
    for (let d of balls) {
        d.draw();
    }
}
