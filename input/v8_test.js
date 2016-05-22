/**
 * V8 Proof of concept with classes
 */
var fps           = 25;
var max_frames    = 10 * fps; // seconds
var realtime      = false;
var scale         = 2;
var canvas_w      = 1920;
var canvas_h      = 1080;

var grad = new gradient();

function initialize() {
    grad.add(0.0, new color(1, 0, 0, 0));
    grad.add(0.2, new color(0, 0, 1, 0));
    grad.add(1.0, new color(0, 0, 0, 0));
}

function next() {
    var index = current_frame / 250.0;
    // TODO: get color object working.. 
    var c = grad.get3(index);
    var r = c.r;
    var g = c.g;
    var b = c.b;
    var a = c.a;
    output("r = " + r );
    var tmp  = new gradient();
    tmp.add(0.0, new color(r, g, b, a))
    tmp.add(0.9, new color(r, g, b, a))
    tmp.add(1.0, new color(0, 0, 0, 0));

    add_circle(new circle(new pos(0, 0, 0), 300, 50, tmp)); 

    add_circle(new circle(new pos(0, 0, 0), current_frame, current_frame / 5., grad)); 
    write_frame();
}

