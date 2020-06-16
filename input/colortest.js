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

function create_gradient(r, g, b) {
    const tmp = new gradient();
    tmp.add(0.0, new color(r, g, b, 1))
    tmp.add(0.9, new color(r, g, b, 1))
    tmp.add(1.0, new color(r, g, b, 0));
    return tmp;
}

function next() {
    add_circle(new circle(new pos(-700, 0, 0), 0, 100, create_gradient(1, 0, 0)));
    add_circle(new circle(new pos(   0, 0, 0), 0, 100, create_gradient(0, 1, 0)));
    add_circle(new circle(new pos(+700, 0, 0), 0, 100, create_gradient(0, 0, 1)));
}

