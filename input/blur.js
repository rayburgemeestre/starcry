/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
const fps           = 25;
const max_frames    = 10 * fps; // seconds
const realtime      = false;
const begin         = +new Date();
const canvas_w      = 1920;
const canvas_h      = 1080;
const scale         = 1;

function create_gradient() {
    const tmp = new gradient();
    tmp.add(0.0, new color(1, 0, 0, 1))
    tmp.add(0.9, new color(1, 0, 0, 1))
    tmp.add(1.0, new color(0, 1, 0, 0));
    return tmp;
}
function next() {
    // ranges -700 to +700 in 25 frames
    var x = -700 + (((700*2) / 25.0) * (current_frame % 25));

    let max = 50; // user configurable motion->GetLogX();
    let maxexp = Math.log(max + 1.0) / Math.log(2.0);

    let linear = (x + 700) / (700 * 2);
    let expf = ((Math.pow(2.0, (linear) * maxexp)) - 1.0) / max;

    let maxpow = Math.pow(2.0, maxexp);
    let logn = (maxpow - (Math.pow(2.0, (1.0 - linear) * maxexp))) / max;

    // output("OK " + x + " -> " + linear + " " + expf + " " + logn);
    add_circle(new circle(new pos(-700 + (linear * (700*2)), -300, 0), 0, 100, create_gradient()));
    add_circle(new circle(new pos(-700 + (logn   * (700*2)), 0,    0), 0, 100, create_gradient()));
    add_circle(new circle(new pos(-700 + (expf   * (700*2)), +300, 0), 0, 100, create_gradient()));
}

