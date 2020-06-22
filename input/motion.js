/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
include('lib/vectors.js')
include('lib/quadtree.js')
include('lib/collisions.js')
include('lib/projection.js')

const fps = 60;
const max_frames = 2 * fps;
const canvas_w = 1920 / 2;
const canvas_h = 1080 / 2;
const scale = 1;

const resolution = 0.1;
const num_balls = 200;
const ball_radius = 5.;

let balls = [];

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
        this.last_collide = -1;
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
    for (let i=0; i<num_balls; i++) {
        balls.push(new ball(view));
    }
}

function next()
{
    set_background_color(new color(0, 0, 0, 255));

    let boundary = new rectangle(new pos(-1 * canvas_w/2.0, -1 * canvas_h/2.0, 0), canvas_w, canvas_h, new gradient());
    let quadtree1 = new quadtree(boundary, 1);

    for (let i=0; i<balls.length; i++) {
        balls[i].move();
        quadtree1.insert(new point(balls[i].circle_, i));
    }

    for (let ball1index=0; ball1index<balls.length; ball1index++) {
        let ball1 = balls[ball1index];
        let ball1shape = ball1.circle_;
        let others = quadtree1.query(new circle(new pos(ball1shape.x, ball1shape.y, 0), ball1shape.radius, ball1shape.radius_size, new gradient()));
        for (let j=0; j<others.length; j++) {
            let ball2index = others[j].userdata;
            let ball2 = balls[ball2index];
            let ball2shape = ball2.circle_;

            if (ball2index <= ball1index) {
                continue;
            }
            // Following is handled by the quad tree directly now..
            //if (!circles_collide(ball1shape, ball2shape)) {
            //    continue;
            //}
            let already_collided = ball2.last_collide == ball1index || ball1.last_collide == ball2index;
            if (already_collided) {
                continue;
            }
            // Excerpt from http://www.gamasutrballs[i].com/view/feature/3015/pool_hall_lessons_fast_accurate_.php?page=3
            // (got mine from http://wonderfl.net/c/rp7P)
            const normal = unit_vector(subtract_vector(ball1shape.as_vec2d(), ball2shape.as_vec2d()));
            const ta = dot_product(ball1.velocity, normal);
            const tb = dot_product(ball2.velocity, normal);
            const optimized_p = (2.0 * (ta - tb)) / 2.0;

            ball1.velocity = subtract_vector(ball1.velocity, multiply_vector(normal, optimized_p));
            ball2.velocity = add_vector(ball2.velocity, multiply_vector(normal, optimized_p));
            ball1.last_collide = ball2index;
            ball2.last_collide = ball1index;
        }
    }

    for (let d of balls) {
        d.draw();
    }
}
