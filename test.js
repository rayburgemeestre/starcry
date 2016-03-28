
// try this for example with  (while true; do date; sleep 0.33333; done & k=$!; sleep 5; kill -9 $k) | ./starcry

var previous_second = null;
var lines = [];

function input(s)
{
    var timestamp = s.split(' ')[3]; // s= 'ma mrt 28 17:39:09 CEST 2016'
    var hms = timestamp.split(':'); // timestamp = '17:39:09'

    if (previous_second !== hms[2] && previous_second !== null) {
        output('rcvd input: ' + lines.join(', '));
        lines = [];
    }
    previous_second = hms[2];

    lines.push(s);
}

function close()
{
    output('Last input! Flush!');
    output('remaining lines in buffer were: ' + lines.join(', '));
}

// another rendering test..

function radius()
{
    //return ((current_frame() * 0.1) + (Math.random() / 10.0)) % 120;
    return Math.abs(120 - current_frame()) % 120;
}
