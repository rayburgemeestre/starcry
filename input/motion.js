/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
const fps = 25;
const max_frames = -1;
const canvas_w = 1920;
const canvas_h = 1080
const scale = 1;

let balls = [];

class vector2d {
    constructor() {
        this.x = 1;
        this.y = 0;
    }
    rotate(degrees)
    {
        const radian = this.degrees_to_radian(degrees);
        const sine = Math.sin(radian);
        const cosine = Math.cos(radian);
        this.x = this.x * cosine - this.y * sine;
        this.y = this.x * sine + this.y * cosine;
    }
    degrees_to_radian(degrees)
    {
        const pi = 3.14159265358979323846;
        return degrees * pi / 180.0;
    }
}

class simple_shape
{
    constructor()
    {
        this.velocity = new vector2d();
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
    constructor() {
        super();
        this.circle_ = new circle(new pos(0, 0, 0), 0, 10.0, this.gradient_);
        this.circle_.blending_type = blending_type.add;
        this.circle_.x = Math.random() * 1000 - 500;
        this.circle_.y = Math.random() * 1000 - 500;
    }
    move() {
        this.circle_.x += this.velocity.x * 20;
        this.circle_.y += this.velocity.y * 20;
        while (this.circle_.x < -500) this.circle_.x += 1000;
        while (this.circle_.y < -500) this.circle_.y += 1000;
        while (this.circle_.x > 500) this.circle_.x -= 1000;
        while (this.circle_.y > 500) this.circle_.y -= 1000;
    }
    draw() {
        add_circle(this.circle_);
    }
}

function initialize()
{
    for (let i=0; i<20; i++) {
        balls.push(new ball());
    }
}

function next()
{
    for (let d of balls) {
        d.move();
        d.draw();
    }
}
