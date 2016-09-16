/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * V8 Proof of concept with classes
 */
var fps           = 25;
var max_frames    = 10 * fps; // seconds
var realtime      = false;
var scale         = 2;
//var canvas_w      = 1920;
//var canvas_h      = 1080;
var canvas_w      = 3840;
var canvas_h      = 2160;

function angle(x, y, x2, y2)
{
    var dx = x - x2,
        dy = y - y2;

    if (dx == 0 && dy == 0)
        return 0;

    if (dx == 0) {
        if (dy < 0)
            return 270;
        else
            return 90;
    }

    var slope = dy / dx;
    var angle = Math.atan(slope);
    if (dx < 0)
        angle += Math.PI;

    angle = 180.0 * angle / Math.PI;

    while (angle < 0.0)
        angle += 360.0;

    return angle;
}

function distance(num, num2) {
    return Math.sqrt((num - num2) * (num - num2));
}

function move_plus(c, angle, rotate, move) {
    var tmpAngle = angle + rotate; // go left...
    if (tmpAngle > 360.0)
        tmpAngle -= 360.0;
    var rads = tmpAngle * Math.PI / 180;
    return new pos(c.x + move * Math.cos(rads), c.y + move * Math.sin(rads), 0);
}

function move_minus(c, angle, rotate, move) {
    var tmpAngle = angle + rotate; // go left...
    if (tmpAngle < 0.0)
        tmpAngle += 360.0;
    var rads = tmpAngle * Math.PI / 180;
    return new pos(c.x + move * Math.cos(rads), c.y + move * Math.sin(rads), 0);
}

function move(c, angle, rotate, move) {
    if (rotate >= 0)
        return move_plus(c, angle, rotate, move);
    else
        return move_minus(c, angle, rotate, move);
}

var grad = new gradient();
var circles = [];

function initialize() {
    grad.add(0.0, new color(1, 1, 1, 0));
    grad.add(0.05, new color(0, 1/10., 0, 0));
    grad.add(1.0, new color(0, 0, 0, 0));
}

function add(x, y, rad, si) {
    for (var i=0; i<7; i++) {
        var dist = distance(x, y);
        var p = new pos(0, 0, 0);
        var p2 = move({x:0, y:0}, (360 / 7) * i - 90, 0, dist * 1.);
        var newcircl = new circle(p2, rad, si, grad);
        //if (i == 0) continue;
        add_circle(newcircl);
    }
}

function next() {
    current_frame_ = 50;
    /*
    circles = [
        new circle(new pos(0, 0, 0), 48, 5, grad),
        new circle(new pos(0, -101, 0), 15, 5, grad),
        new circle(new pos(0, -206, 0), 38, 5, grad),
        new circle(new pos(0, -411, 0), 119, 5, grad)
    ];
    */

    var extra = 250. - current_frame, // stupid animation
        rs    = 100;

    // debug, add_circle(new circle(new pos(0, 0, 0), current_frame, 10, grad)); // center circle

    //add_text(0, 0, 0, 'This is frame ' + current_frame, 'center');
    add_circle(new circle(new pos(0, 0, 0), 48+extra, rs, grad)); // center circle
    add(0, -101, 15+extra, rs); // first ring of 7
    add(0, -206, 38+extra, rs); // second ring of 7
    add(0, -411, 119+extra, rs); // third ring of 7

    write_frame();
}

