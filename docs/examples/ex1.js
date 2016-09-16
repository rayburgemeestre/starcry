var fps           = 25;
var max_frames    = 10 * fps; // 10 seconds
var canvas_w      = 480;
var canvas_h      = 320;

function next() {
    var tmp = new gradient();
    tmp.add(0.0, new color(1, 0, 0, 1))
    tmp.add(1.0, new color(0, 0, 0, 0));

    var radius = current_frame;
    add_circle(new circle(new pos(0, 0, 0), radius, 5.0, tmp));
    write_frame();
}
