/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
var fps           = 25;
var max_frames    = -1;
var canvas_w      = 1920;
var canvas_h      = 1080;

var clock_max_radius = Math.min(canvas_w, canvas_h) / 2.0 - 50.;
var circles = [], second = 0, minute = 1, hour = 2;

function generate_gradient(r, g, b)
{
    var grad = new gradient();
    grad.add(0.0, new color(r, g, b, 1))
    grad.add(0.5, new color(r, g, b, 1));
    grad.add(1.0, new color(r, g, b, 0));
    return grad;
}
var yellow_gradient = generate_gradient(1, 1, 0);
var lightgrey_gradient = generate_gradient(0.1, 0.1, 0.1);
var grey_gradient = generate_gradient(0.2, 0.2, 0.2);
var blue_gradient = generate_gradient(0, 0, 1);

function initialize()
{
    circles.push(new circle(new pos(0, 0, 0), 0, 2., yellow_gradient));
    circles.push(new circle(new pos(0, 0, 0), 0, 5.0, blue_gradient));
    circles.push(new circle(new pos(0, 0, 0), 0, 10.0, blue_gradient));

    var type = blending_type.screen;
    circles[second].blending_type = type;
    circles[minute].blending_type = type;
    circles[hour].blending_type = type;

    for (var i=1; i<12; i++) {
        var s = ((clock_max_radius / 12) * i);
        circles.push(new circle(new pos(0, 0, 0), s, 2.0, lightgrey_gradient));
    }
    circles.push(new circle(new pos(0, 0, 0), clock_max_radius, 15.0, grey_gradient));
}

function next() {
    var d = new Date();
    var m_hour = d.getHours() % 12;
    var m_min = d.getMinutes();
    var m_sec = d.getSeconds();
    var m_ms = d.getMilliseconds();
    var rad_sec = ((m_sec * 1000) + (m_ms)) / 1000 / 60;
    var rad_min = ((m_min * 60 * 1000) + (m_sec * 1000) + (m_ms)) / (60 * 1000) / 60;
    var rad_hour = (((m_hour * 60 * 60 * 1000) + (m_min * 60 * 1000) + (m_sec * 1000) + (m_ms)) / (60 * 60 * 1000) / 12);
    var second_radius = Math.max((1.0 - rad_sec) * 5, 2.0);
    
    circles[hour].radius = rad_hour * clock_max_radius;
    circles[minute].radius = rad_min * clock_max_radius;
    circles[second].radius = rad_sec * clock_max_radius;
    circles[second].radius_size = second_radius;

    set_background_color(new color(0, 0, 0, 1));

    for (var i=circles.length - 1; i>=0; i--) {
        add_circle(circles[i]);
    }
    for (var i=1; i<=12; i++) {
        add_text(((clock_max_radius / 12) * i), 0, 0, 10, '' + i, 'center');
    }
    add_text(0, 30 - canvas_h /2, 0, 30, 'Time ' + new Date(), 'center');
}
