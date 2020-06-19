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
var bitrate       = 10 /* megabyte */ * 1024 /* kilobyte */ * 1024 /* bytes */ * 8 /* bits */;
var realtime      = false;
var canvas_w      = 3840;
var canvas_h      = 2160;
var scale         = 1;
//var canvas_w      = 1000;
//var canvas_h      = 1000;
//var scale         = 0.5;

// canvas_w /= 4;
// canvas_h /= 4;
// scale /= 4;

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
    grad.add(0.0, new color(1, 1, 1, 1));
    grad.add(0.05, new color(0, 1/10., 0, 1));
    grad.add(1.0, new color(0, 0, 0, 0));
}

var type = blending_type.add;

function add(extra, rs, level, xp, yp, x, y, rad, si) {
    for (var i=0; i<7; i++) {
        var dist = distance(x, y);
        var p = new pos(0, 0, 0);
        var p2 = move({x:xp, y:yp}, (360 / 7) * i - 90, 0, dist * 1.);
        var newcircl = new circle(p2, rad, si, grad);
        bright_circles(extra, rs, p2.x, p2.y, level + 1, rad);
        newcircl.blending_type = type;
        //if (i == 0) continue;
        add_circle(newcircl);
    }
}

function bright_circles(extra, rs, xp, yp, level, scale2)
{
    level = level || 0;
    if (typeof scale2 == 'undefined') {
        scale2 = 1.0;
    } else {
//        var tmp = (250 - extra) / 250.0;
//        tmp *= 350;
//        tmp += 100;
//        // want: tween 100 - 450
//        scale2 = scale2 / tmp;//450;
        var rat = current_frame / max_frames;
        scale2 = scale2 / (50 + (400 * rat));
    }
    if (level >= 2) return;

    var rad1 = 48 * scale2;
    var rad2 = 15 * scale2;
    var rad3 = 38 * scale2;
    var rad4 = 119 * scale2;

    var c1 = new circle(new pos(xp, yp, 0), (rad1+extra) * scale2, rs, grad);
    c1.blending_type = type;
    add_circle(c1); // center circle
    bright_circles(extra, rs, xp, yp, level + 1, (rad1+extra) * scale2);
    add(extra, rs, level, xp, yp, 0, -101 * scale2, (rad2+extra) * scale2, rs); // first ring of 7
    add(extra, rs, level, xp, yp, 0, -206 * scale2, (rad3+extra) * scale2, rs); // second ring of 7
    add(extra, rs, level, xp, yp, 0, -411 * scale2, (rad4+extra) * scale2, rs); // third ring of 7
}

function next()
{
    current_frame_ = 50;
    /*
    circles = [
        new circle(new pos(0, 0, 0), 48, 5, grad),
        new circle(new pos(0, -101, 0), 15, 5, grad),
        new circle(new pos(0, -206, 0), 38, 5, grad),
        new circle(new pos(0, -411, 0), 119, 5, grad)
    ];
    */
    // solve this crap with unit tests, and just add this functionality to the color objects!
    set_background_color(new color(0, 0, 0, 1));

    var extra = 0;//250. - current_frame, // stupid animation
        rs    = 100;

    // debug, add_circle(new circle(new pos(0, 0, 0), current_frame, 10, grad)); // center circle

    //add_text(0, 0, 0, 'This is frame ' + current_frame, 'center');

    var y = 0;
    var x = 0;
    bright_circles(extra, rs, x, y);
}

