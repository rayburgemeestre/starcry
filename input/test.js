/**
 * Generate video where circle radius increases by time.
 */
var fps           = 25;
var max_frames    = 10 * fps; // seconds
var realtime      = true;
var seconds       = 0;
var begin         = +new Date();

function next() {
    var diff = +new Date() - begin;
    seconds = (diff / 1000.0) % 60;
    add_circle(new circle(new pos(0, 0, 0), seconds, 5.0, new color(1, 1, 1, 0)));
    write_frame();
}
