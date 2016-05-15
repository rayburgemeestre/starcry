/**
 * V8 Proof of concept with classes
 */
var fps           = 25;
var max_frames    = 10 * fps; // seconds
var realtime      = false;
var scale         = 1;
var canvas_w      = 1920;
var canvas_h      = 1080;

var glob = new Y(1.0);

function initialize() {
}

function next() {
//    var c = new color(1, 0, 1, 1);
//    var l = new line(0, 0, 0, 10, 10, 10);
//    output('v2 = ' + l.x2);

//    var radius      = current_frame / 10;
//    var radius_size = 5.0 + current_frame / 2;
//    //var s1 = new circle(10, 10, 0, radius, radius_size);
//    var s2 = new circle(new pos(0, 0, 0), radius, radius_size, new color(1, 0, 1, 1));
    //add_circle(0, 0, 0, radius, radius_size);
    //add_circle2(s1);
//    add_circle2(new circle(new pos(0, 0, 0), radius, radius_size, new color(0, 0, 1, 1)));
//    add_circle2(new circle(new pos(-10, 0, 0), radius, 2, new color(1, 0, 0, 1)));
//    add_circle2(new circle(new pos(0, 0, 0), radius, 2, new color(1, 0, 0, 1)));
//    add_circle2(new circle(new pos(10, 0, 0), radius, 2, new color(1, 0, 0, 1)));
    for (var i=0; i< 100;i ++) {
        add_circle(new circle(new pos(0, 0, 0), (i * 15) + (current_frame), 3, new color(1, 0, 0, 1)));
    }
    write_frame();
}

