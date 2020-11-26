/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
'use strict';

Number.prototype.pad = function(size) {
  let s = String(this);
  while (s.length < (size || 2)) {
    s = '0' + s;
  }
  return s;
};

const fps = 25;
const max_frames = -1;
const canvas_w = 1920;
const canvas_h = 1080;
const scale = 1;
const bitrate = 1000000;
const realtime = true;

const clock_max_radius = Math.min(canvas_w, canvas_h) / 2.0 - 50.;
const circles = [], second = 0, minute = 1, hour = 2;

function generate_gradient(r, g, b) {
  const grad = new gradient();
  grad.add(0.0, new color(r, g, b, 1))
  grad.add(0.5, new color(r, g, b, 1));
  grad.add(1.0, new color(r, g, b, 0));
  return grad;
}

function generate_wide_gradient(r, g, b) {
  const grad = new gradient();
  grad.add(0.0, new color(r, g, b, 1))
  grad.add(0.05, new color(r, g, b, 0.4));
  grad.add(1.0, new color(r, g, b, 0));
  return grad;
}

function initialize() {
  circles.push(new circle(new pos(0, 0, 0), 0, 1.5, generate_gradient(1, 1, 0)));
  circles.push(new circle(new pos(0, 0, 0), 0, 80.0, generate_wide_gradient(0, 0, 1)));
  circles.push(new circle(new pos(0, 0, 0), 0, 160.0, generate_wide_gradient(0, 0, 1)));

  const type = blending_type.add;
  circles[second].blending_type = type;
  circles[minute].blending_type = type;
  circles[hour].blending_type = type;
}

function next() {
  const d = new Date();
  const m_hour = d.getHours() % 12;
  const m_min = d.getMinutes();
  const m_sec = d.getSeconds();
  const m_ms = d.getMilliseconds();
  const rad_sec = ((m_sec * 1000) + (m_ms)) / 1000 / 60;
  const rad_min = ((m_min * 60 * 1000) + (m_sec * 1000) + (m_ms)) / (60 * 1000) / 60;
  const rad_hour =
      (((m_hour * 60 * 60 * 1000) + (m_min * 60 * 1000) + (m_sec * 1000) + (m_ms)) / (60 * 60 * 1000) / 12);
  circles[hour].radius = rad_hour * clock_max_radius;
  circles[minute].radius = rad_min * clock_max_radius;
  circles[second].radius = rad_sec * clock_max_radius;
  set_background_color(new color(0, 0, 0, 1));
  for (let i = circles.length - 1; i >= 0; i--) {
    add_circle(circles[i]);
  }
  for (let i = 1; i <= 12; i++) {
    add_text(((clock_max_radius / 12) * i), 0, 0, 20, '' + i, 'center');
    add_text(((clock_max_radius / 12) * i), 30, 0, 20, '' + (5 * i), 'center');
  }
  const time_str = 'Time ' + m_hour.pad(2) + ':' + m_min.pad(2) + ':' + m_sec.pad(2) + '.' + m_ms.pad(3);
  add_text(0, 30 - canvas_h / 2, 0, 20, time_str, 'center');
}
