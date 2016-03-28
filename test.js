
var shapes = [];

function init()
{
    set_canvas(320, 240);
    for (var i=0; i<10; i++) {
        shapes.add(new circle(0, 0, 0, 100, 2.0));
    }
}

var line = '';
function input(s) {
    line = s;
    return 1123;
}

function test()
{
    return 'line';
}

// test
function radius()
{
    //return ((current_frame() * 0.1) + (Math.random() / 10.0)) % 120;
    return Math.abs(120 - current_frame()) % 120;
}
