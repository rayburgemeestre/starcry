/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

var vector2d = class
{
    constructor(x = 0, y = 0) {
        this.x = x;
        this.y = y;
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

function add_vector(a, b)
{
    return new vector2d(a.x + b.x, a.y + b.y);
}

function subtract_vector(a, b)
{
    return new vector2d(a.x - b.x, a.y - b.y);
}

function divide_vector(v, d)
{
    return new vector2d(v.x / d, v.y / d);
}

function multiply_vector(v, s)
{
    return new vector2d(v.x * s, v.y * s);
}

function dot_product(a, b)
{
    return a.x * b.x + a.y * b.y;
}

function vector_length(v)
{
    return Math.sqrt(dot_product(v, v));
}

function unit_vector(v)
{
    let length = vector_length(v);
    if (length != 0)
        return divide_vector(v, length);
    return v;
}
