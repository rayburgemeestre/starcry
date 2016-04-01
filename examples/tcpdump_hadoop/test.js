/**
 * Example that converts tcpdump data for a specific Hadoop cluster into a visualization.
 *
 * use: `cat tcpdump_all.txt| ./starcry --stdin -d 1920x1080 -n 8 -c 1`
 */

var fps           = 25;
var max_frames    = 99995 * fps; // seconds (TODO: I need to start supporting null or 0)
var realtime      = false;

// nodes and their IP's in this cluster.
var nodes = {
    'headnode' : '10.141.255.254',
    'node1' : '10.141.0.1',
    'node2' : '10.141.0.2',
    'node3' : '10.141.0.3'
};

// nodes with ports used by specific hadoop process
var info = [
    ['node1', 50475, 'datanode'],
    ['node1', 10033, 'historyserver'],
    ['node1', 8019, 'zkfc'],
    ['node1', 8020, 'namenode'],
    ['node1', 40832, 'datanode'],
    ['node1', 8481, 'journalnode'],
    ['node1', 50020, 'datanode'],
    ['node1', 10020, 'historyserver'],
    ['node1', 8485, 'journalnode'],
    ['node1', 50470, 'namenode'],
    ['node1', 8040, 'nodemanager'],
    ['node1', 3888, 'zookeeper'],
    ['node1', 8090, 'resourcemanager'],
    ['node1', 13562, 'nodemanager'],
    ['node1', 38077, 'zookeeper'],
    ['node1', 8190, 'timelineserver'],
    ['node1', 16000, 'KMS'],
    ['node1', 8033, 'resourcemanager'],
    ['node1', 16001, 'KMS'],
    ['node1', 10021, 'timelineserver'],
    ['node1', 2181, 'zookeeper'],

    ['node2', 50475, 'datanode'],
    ['node2', 10033, 'historyserver'],
    ['node2', 8019, 'zkfc'],
    ['node2', 8020, 'namenode'],
    ['node2', 53824, 'datanode'],
    ['node2', 8481, 'journalnode'],
    ['node2', 50020, 'datanode'],
    ['node2', 10020, 'historyserver'],
    ['node2', 8485, 'journalnode'],
    ['node2', 50470, 'namenode'],
    ['node2', 8040, 'nodemanager'],
    ['node2', 3888, 'zookeeper'],
    ['node2', 8090, 'resourcemanager'],
    ['node2', 13562, 'nodemanager'],
    ['node2', 60347, 'zookeeper'],
    ['node2', 48156, 'nodemanager'],
    ['node2', 8030, 'resourcemanager'],
    ['node2', 8190, 'timelineserver'],
    ['node2', 8031, 'resourcemanager'],
    ['node2', 8033, 'resourcemanager'],
    ['node2', 10021, 'timelineserver'],
    ['node2', 2181, 'zookeeper'],

    ['node3', 50475, 'datanode'],
    ['node3', 8481, 'journalnode'],
    ['node3', 50020, 'datanode'],
    ['node3', 8485, 'journalnode'],
    ['node3', 8040, 'nodemanager'],
    ['node3', 2888, 'zookeeper'],
    ['node3', 8044, 'nodemanager'],
    ['node3', 3888, 'zookeeper'],
    ['node3', 13562, 'nodemanager'],
    ['node3', 55519, 'zookeeper'],
    ['node3', 2181, 'zookeeper']
];

// used to calculate what frame we're at etc.
var offsetDate       = null;
var currentTime      = null;
var currentFrame     = null;
var writtenFrame     = 0;

var info_text        = '';
var lines            = [];
var console_         = [];
var map              = {};
var ipToNode         = {};
var display_label    = {};

function initialize()
{
    // generate a convenient map for looking up data for any node + port
    // data can be number of bytes received/send to another node, and helper values.
    for (var i=0; i<info.length; i++) {
        var data       = info[i];
        var node       = data[0];
        var port       = data[1];
        var service    = data[2];
        for (var node2 in nodes) {
            if (!map[node]) map[node] = {};
            if (!map[node][port]) map[node][port] = {};
            if (!map[node][port][node2]) map[node][port][node2] = [];
            map[node][port][node2][0] = 0; // reset data_len to 0
            map[node][port][node2][1] = service;
            map[node][port][node2][2] = 0; // reset data_len for input to 0
            map[node][port][node2][3] = 0; // not used
            display_label[node + ' ' + port] = 0;
        }
    }
    // useful mapping to get node by ip
    for (var node in nodes) {
        var ip = nodes[node];
        ipToNode[ip] = node;
    }
}

function process()
{
    // iterate number of frames we need to still write out / catch up with
    while (writtenFrame < currentFrame) {
        // determine location on screen for nodes
        var node_xy = {};
        for (var node in nodes) {
            var x = -150, y = -120;
            if (node === 'node1') x += -100;
            else if (node === 'node2') x += 0;
            else if (node === 'node3') x += 100;
            else if (node === 'headnode') y = +80;
            else continue;
            node_xy[node] = [x, y];
        }

        var lbls    = [];
        var lns     = [];
        ////// updating..
        //for (var i=0; i<display_label.length; i++) {
        //    display_label[i] -= 0.1;//0.2 / 25 / 10; // used
        //    if (display_label[i] <= 0) {
        //        //display_label[i] = 0;
        //    }
        //    //display_label[i] = 0; // test
        //}

        // update all the data in the map, i.e., as time elapses some draw values decrease etc.
        for (var node in nodes) {
            if (!node_xy[node] || typeof node_xy[node] === 'array')
                continue;
            for (var dst in map) {
                if (dst !== node) continue;
                for (var dst_port in map[dst]) {
                    for (var src in map[dst][dst_port]) {
                        var a = function (idx, r, g, b, offset) {
                            var greyness = map[dst][dst_port][src][3];
                            if (map[dst][dst_port][src][idx] >= 10000)
                                map[dst][dst_port][src][idx] /= 1.5;
                            else
                                map[dst][dst_port][src][idx] -= 100;
                            if (map[dst][dst_port][src][idx] < 0)
                                map[dst][dst_port][src][idx] = 0

                        };
                        a(0, 0, 1.0, 0, -35); // input
                        a(2, 1.0, 0, 0, -35); // output
                        map[dst][dst_port][src][3] -= 0.2 / 25 / 10; // used
                        if (map[dst][dst_port][src][3] <= 0) {
                            map[dst][dst_port][src][3] = 0;
                        }
                    }
                }
            }
        }

        // draw everything on the screen, some stuff is buffered first, so we can control which order to draw elements
        for (var node in nodes) {
            if (!node_xy[node] || typeof node_xy[node] === 'array')
                continue;
            var x = node_xy[node][0];
            var y = node_xy[node][1];

            var index = 20;
            for (var dst in map) {
                if (dst !== node) continue;
                for (var dst_port in map[dst]) {
                    for (var src in map[dst][dst_port]) {
                        var a = function (idx, r, g, b, offset) {
                            var greyness = map[dst][dst_port][src][3];
                            if (map[dst][dst_port][src][idx] > 0) {
                                lns.push([
                                    node_xy[src][0],
                                    node_xy[src][1],
                                    0,
                                    x + offset,
                                    y + index,
                                    0,
                                    src === 'headnode' ? 10.0 : 5.0 + (map[dst][dst_port][src][idx] / 1000.0),
                                    r,
                                    g,
                                    b
                                ]);
                                display_label[dst + ' ' + dst_port] = 0.2;
                            }
                            else if (greyness > 0) {
                                add_line(
                                    node_xy[src][0],
                                    node_xy[src][1],
                                    0,
                                    x + offset,
                                    y + index,
                                    0,
                                    2.0,
                                    greyness,
                                    greyness,
                                    greyness
                                );
                                display_label[dst + ' ' + dst_port] = greyness;
                            }
                        };
                        a(0, 0, 1.0, 0, -35); // input
                        a(2, 1.0, 0, 0, -35); // output
                    }
                    var lookup = dst + ' ' + dst_port;
                    lbls.push([
                        x,
                        y + index,
                        0,
                        dst_port + ' - ' + map[dst][dst_port][src][1],
                        lookup,
                        (display_label[lookup] > 0)
                    ]);
                    index += 7;
                }
            }
        }
        var len = 30;
        for (var i=0; i<lbls.length; i++){
            var l = lbls[i];;
            if (display_label[l[4]] <= 0.001)
                continue;
            if (l[5]) {
                add_line(l[0]-len - 5, l[1], 0, l[0]+len /*+ 5*/, l[1], 0, 2, 0, 0.5, 0);
                add_line(l[0]-len, l[1], 0, l[0]+len, l[1], 0, 25, 0, 0.5, 0);
            }
            add_text(l[0], l[1], l[2], l[3]);
        }
        for (var node in nodes) {
            if (!node_xy[node] || typeof node_xy[node] === 'array')
                continue;
            var x = node_xy[node][0];
            var y = node_xy[node][1];
            var len = 15;
            add_line(x-len, y, 0, x+len, y, 0, 25, 0.5, 0, 0);
            add_text(x, y, 0, node);
        }
        for (var i=0; i<lns.length; i++){
            var l = lns[i];;
            add_line(l[0], l[1], l[2], l[3], l[4], l[5], l[6], l[7], l[8], l[9]);
        }

        var x = +190, y = -150;
        for (var i=0; i<console_.length; i++) {
            add_text(x, y, 0, '' + console_[i]);
            y += 10;
        }

        add_text(-190, -150, 0, info_text);

        write_frame();

        writtenFrame++;
    }

    // process the (tcpdump/job output) lines in buffer
    for (var i=0; i<lines.length; i++) {
        var line        = lines[i];
        var src         = ipToNode[line[0]];
        var src_port    = line[1];
        var dst         = ipToNode[line[2]];
        var dst_port    = line[3];
        var data_len    = parseInt(line[4]);
        try {
            map[dst][dst_port][src][0] += data_len;
            map[dst][dst_port][src][3] = 0.2; // used
        }
        catch (e) {
            try {
                map[src][src_port][dst][2] += data_len;
                map[src][src_port][dst][3] = 0.2; //used
            }
            catch (e) {
                output('problem = ' + src + ' ' + src_port + ' ' + dst + ' ' + data_len);
            }
        }
    }
    // clear buffer
    lines = [];
}

function input(line) {
    // this regex matches tcpdump lines of the following format
    // '20:57:40.108039 IP 10.141.255.254.36107 > 10.141.0.2.8020: tcp 85'
    var matches = line.match(/(\d+):(\d+):(\d+).(\d+) IP (\d+.\d+.\d+\.\d+).(\d+) > (\d+.\d+.\d+\.\d+).(\d+): tcp (\d+).*/ );
    // [
    // 0  "20:57:40.108039 IP 10.141.255.254.36107 > 10.141.0.2.8020: tcp 85",
    // 1  "20",
    // 2  "57",
    // 3  "40",
    // 4  "108039",
    // 5  "10.141.255.254",
    // 6  "36107",
    // 7  "10.141.0.2",
    // 8  "8020",
    // 9  "85"
    // ]
    var matches2 = null;
    if (matches === null) {
        // this regex matches job output lines
        matches2 = line.match(/(\d+):(\d+):(\d+).(\d+) @@job@@ (.*)/);
        if (matches2 === null) {
            output("WRONG INPUT!\n");
            return;
        }
        matches = matches2;
    }

    // read time from the given line
    var time = matches[1] + ':' + matches[2] + ':'  + matches[3];
    var dt = new Date('2016-01-01 ' + time);
    var frame = Math.floor(parseInt(matches[4]) / (1000000 / fps));
    if (currentTime === null) {
        currentTime     = time;
        offsetDate      = dt;
        currentFrame    = frame;
    }

    // this block is for handling lines from job output
    if (matches2) {
        // header is shown top-left
        if (matches[5].startsWith('INFO')) {
            info_text = matches[5].substr(5);
            return;
        }
        // buffer console messages
        console_.push(matches[5]);
        console_ = console_.slice(-30);
        return;
    }

    // this block is for handling tcpdump lines, they are buffered in lines array.
    frame += parseInt(((dt - offsetDate) / 1000) * 25);
    if (currentFrame != frame) {
        process();
        currentFrame = frame;
    }
    lines.push([matches[5], matches[6], matches[7], matches[8], matches[9]]);
}

function close() {
    output('no more input.. calling process() to handle everything still buffered..');
    process();
}

function next() {}
