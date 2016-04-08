var data = [
    ["node1", 10903, "datanode", 50010],
    ["node1", 10903, "datanode", 50020],
    ["node1", 10903, "datanode", 56663],
    ["node1", 11199, "KMS", 16000],
    ["node1", 11199, "KMS", 16001],
    ["node1", 11441, "historyserver", 10020],
    ["node1", 11441, "historyserver", 19888],
    ["node1", 11519, "namenode", 8020],
    ["node1", 11680, "nodemanager", 13562],
    ["node1", 11680, "nodemanager", 44210],
    ["node1", 11680, "nodemanager", 8040],
    ["node1", 11984, "resourcemanager", 8030],
    ["node1", 11984, "resourcemanager", 8033],
    ["node1", 11984, "resourcemanager", 8088],
    ["node1", 12590, "timelineserver", 10021],
    ["node1", 12590, "timelineserver", 8188],
    ["node2", 15274, "datanode", 50010],
    ["node2", 15274, "datanode", 50020],
    ["node2", 15274, "datanode", 50034],
    ["node2", 15412, "nodemanager", 13562],
    ["node2", 15412, "nodemanager", 8040],
    ["node2", 15614, "secondarynamenode", 50090],
    ["node3", 13183, "datanode", 33677],
    ["node3", 13183, "datanode", 50010],
    ["node3", 13183, "datanode", 50020],
    ["node3", 13331, "nodemanager", 13562],
    ["node3", 13331, "nodemanager", 8040],
    ["headnode", '',  "job", '']
];

data.sort(function(a, b) {
    return a[3] > b[3];
});

var ips = {
    '10.141.0.1'     : 'node1',
    '10.141.0.2'     : 'node2',
    '10.141.0.3'     : 'node3',
    '10.141.255.254' : 'headnode',
};

var fps            = 25;
var max_frames     = 10000000 * fps;
var realtime       = false;
var offset_date    = false;
var first_frame    = false;
var buffer         = [];
var frame          = 0;
var current_frame  = 0;
var previous_frame = 0;
var nodes          = {};

function draw_box(x, y, width, height) {
    var r = 1, g = 0, b = 0;
    width /= 2.0;
    height /= 2.0;

    add_line(x - width, y - height, 0,
             x - width, y + height, 0, 2.0, r, g, b);
    add_line(x - width, y + height, 0,
             x + width, y + height, 0, 2.0, r, g, b);
    add_line(x + width, y - height, 0,
             x + width, y + height, 0, 2.0, r, g, b);
    add_line(x + width, y - height, 0,
             x - width, y - height, 0, 2.0, r, g, b);
}

var coords = {};

function draw_node(node, x, y) {
    var offset = 0;
    for (service in nodes[node]) {

        // service
        var service_x = x;
        var service_y = y + offset;
        add_text(service_x, service_y, 0, service, 'center');

        // ports
        offset += 10;
        var ports = nodes[node][service].ports;
        var len = 0;
        for (port in ports) { len++; }
        var port_offset = -1 * (((len - 1) / 2.0) * 20);
        for (port in ports) {
            var port_x = x + port_offset;
            var port_y = y + offset;
            add_text(port_x, port_y, 0, '' + port, 'center');
            draw_box(port_x, port_y, 18, 6);
            coords[node + port] = { x: port_x, y: port_y };
            port_offset += 20;
        }

        // extra spacing
        offset += 20;

        draw_box(service_x, service_y + 5, 70, 20);
        draw_box(service_x + 40 + 2.5, service_y - 2.5, 5, 5);
        coords[node + service] = { x: service_x + 40 + 2.5, y: service_y - 2.5 };
        draw_box(service_x + 40 - 2.5, service_y - 2.5, 5, 0);

    }
}

var activity = {};
var console = [];
var header = '';

function draw_header(x, y) {
    add_text(x, y, 0, header, 'left');
}

function draw_console(x, y) {
    console = console.slice(-30);
    var offset = 0;
    for (var i=0; i<console.length; i++) {
        add_text(x, y + offset, 0, console[i], 'left');
        offset += 10;
    }
}

function process() {
    if (frame != previous_frame) {
        while (current_frame < frame) {
            for (var a in activity) {
                if (activity[a].bytes > 20) {
                    activity[a].bytes /= 2;
                }
                else {
                    activity[a].bytes -= 1;
                }
                if (activity[a].bytes <= 0) {
                    activity[a].bytes = 0;
                    activity[a].shadow -= (0.2 / (25 * 2));
                }
                if (activity[a].shadow < 0) {
                    activity[a].shadow = 0;
                }
            }

            for (var i=0; i<buffer.length; i++) {
                var line = buffer[i];

                if (line.startsWith('@@job@@')) {
                    line = line.substr(8);
                    if (line.startsWith('INFO'))
                        header = line.substr(5);
                    else
                        console.push(line);
                    continue;
                }

                var c = line.split(' ');
                c[4] = c[4] == '127.0.0.1' ? c[0] : ips[c[4]];
                if (!c[4]) continue;
                var from = c[4] + c[5]; // node + port
                var to   = c[0] + c[1]; // node + service
                if (!(from in activity))
                    activity[from + to] = { 
                        from: from, to: to, bytes: 0, shadow: 0.2 
                    };

                activity[from + to].bytes += c[6];
            }
            buffer = [];
            current_frame++;

            draw_node('node1',    -250, -125);
            draw_node('node2',    -150, -125);
            draw_node('node3',    -50,  -125);
            draw_node('headnode', -150,  125);
            draw_console(10, -125);
            draw_header(-250, -150);

            for (var a in activity) {
                var act = activity[a];
                var from = coords[act.from];
                var to = coords[act.to];
                if (from && to) {
                    var size = act.bytes / 100.0;
                    if (size > 0) {
                        add_line(from.x, from.y, 0, to.x, to.y, 0, size + 5, 0, 1, 0);
                    }
                    else if (act.shadow) {
                        add_line(from.x, from.y, 0, to.x, to.y, 0, 5.0, act.shadow, act.shadow, act.shadow);
                    }
                }
            }

            write_frame();
        }
    }
}

function input(line) {
    var matches = line.match(/(\d+):(\d+):(\d+).(\d+) (.*)/ );
    var time    = matches[1] + ':' + matches[2] + ':'  + matches[3];
    var dt      = new Date('1984-02-23 ' + time); // add a date for Mr. Javascript

    frame       = Math.floor(parseInt(matches[4]) / (1000000 / fps));
    offset_date = offset_date || dt;
    first_frame = first_frame || frame;

    frame += parseInt(((dt - offset_date) / 1000) * 25) - first_frame;

    process();

    buffer.push(matches[5]);
    previous_frame = frame;
}

function initialize() {
    for (var i=0; i<data.length; i++) {
        var node    = data[i][0];
        var pid     = data[i][1];
        var service = data[i][2];
        var port    = data[i][3];

        if (!(node in nodes)) nodes[node] = {};
        if (!(service in nodes[node])) nodes[node][service] = {
            pid : pid,
            ports : {},
        };
        nodes[node][service].ports[port] = {};
    }
}

function close() { frame++; process(); }

