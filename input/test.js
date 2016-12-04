/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
const fps           = 25;
const max_frames    = 10 * fps; // seconds
const realtime      = false;
const seconds       = 0;
const begin         = +new Date();
const canvas_w      = 1920;
const canvas_h      = 1080;
const scale         = 5;

function next() {
    seconds = current_frame;
    const tmp = new gradient();
    tmp.add(0.0, new color(1, 0, 0, 1))
    tmp.add(1.0, new color(0, 1, 0, 0));
    const test = new circle(new pos(0, 0, 0), seconds, 5.0, tmp);
    add_circle(test);
}

