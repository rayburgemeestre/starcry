/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Generate video where circle radius increases by time.

var fps           = 25;
var max_frames    = -1;
var seconds       = 0;
var begin         = +new Date();
var canvas_w      = 1080;
var canvas_h      = 1080;
var scale         = 1;

function next() {
    var d = new Date();
    var m_hour   = d.getHours() % 12;
    var m_min    = d.getMinutes();
    var m_sec    = d.getSeconds();
    var m_ms     = d.getMilliseconds();

    var rad_sec  = ((m_sec * 1000) + (m_ms)) / 1000 / 60;
    var rad_min  = ((m_min * 60 * 1000) + (m_sec * 1000) + (m_ms)) / (60 * 1000) / 60;
    var rad_hour = (((m_hour * 60 * 60 * 1000) + (m_min * 60 * 1000) + (m_sec * 1000) + (m_ms)) / (60 * 60 * 1000) / 12);
    
    set_background_color(new color(0, 0, 1, 1));
    var type = blending_type.normal;
    var tmp = new gradient();
    tmp.add(0.0, new color(1, 0, 0, 1))
    tmp.add(0.5, new color(1, 0, 0, 1));
    tmp.add(1.0, new color(1, 0, 0, 0));
    var tmp2 = new gradient();
    tmp2.add(0.0, new color(1, 1, 1, 1))
    tmp2.add(0.5, new color(1, 1, 1, 1))
    tmp2.add(1.0, new color(1, 1, 1, 0));
//    add_text(0, -50, 0, 100, 'Frame ' + current_frame, 'center');
    add_text(0, 30 - canvas_h /2, 0, 30, 'Time ' + new Date(), 'center');
    var test = new circle(new pos(0, 0, 0), rad_hour * 490, 15.0, tmp);
    test.blending_type = type;
    add_circle(test);
    var test = new circle(new pos(0, 0, 0), rad_min * 490, 7.0, tmp);
    test.blending_type = type;
    add_circle(test);

    var second_radius = ((1.0 - rad_sec) * 5);
    second_radius = Math.max(second_radius, 2.0);
    var test = new circle(new pos(0, 0, 0), rad_sec * 490, second_radius, tmp2);
    test.blending_type = type;
    add_circle(test);
}
