/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scripting.h"
#include "actors/job_generator.h"
#include "util/assistant.h"

using namespace std;

using output_line = atom_constant<atom("output_lin")>;
using write_frame = atom_constant<atom("write_fram")>;

double get_version() {
    return 0.1;
}

void set_background_color(color clr) {
    assistant->the_job.background_color.r = clr.get_r();
    assistant->the_job.background_color.g = clr.get_g();
    assistant->the_job.background_color.b = clr.get_b();
    assistant->the_job.background_color.a = clr.get_a();
}

void add_circle(circle circ) {
    data::shape new_shape;
    new_shape.x = circ.get_x();
    new_shape.y = circ.get_y();
    new_shape.z = circ.get_z();
    new_shape.type = data::shape_type::circle;
    new_shape.radius = circ.get_radius();
    new_shape.radius_size = circ.get_radiussize();
    new_shape.gradient_ = circ.get_gradient().to_data_gradient();
    new_shape.blending_ = circ.blending_type_;
    assistant->the_job.shapes.push_back(new_shape);
}

void add_line(line l) {
    data::shape new_shape;
    new_shape.x = l.get_x();
    new_shape.y = l.get_y();
    new_shape.z = l.get_z();
    new_shape.x2 = l.get_x2();
    new_shape.y2 = l.get_y2();
    new_shape.z2 = l.get_z2();
    new_shape.gradient_ = l.get_gradient().to_data_gradient();
    new_shape.blending_ = l.blending_type_;
    new_shape.type = data::shape_type::line;
    new_shape.radius_size = l.get_size();
    assistant->the_job.shapes.push_back(new_shape);
}

void add_text(double x, double y, double z, double textsize, string text, string align) {
    data::shape new_shape;
    new_shape.x = x;
    new_shape.y = y;
    new_shape.z = z;
    new_shape.text_size = textsize;
    new_shape.type = data::shape_type::text;
    new_shape.text = text;
    new_shape.align = align;
    assistant->the_job.shapes.push_back(new_shape);
}

void output(string s) {
    assistant->job_generator->send(assistant->job_generator, output_line::value, s);
}

void write_frame_fun() {
    write_frame_fun_impl(false);
}

void close_fun() {
    write_frame_fun_impl(true);
}

// deprecated

void write_frame_fun_impl(bool last_frame) {
    if (!assistant->the_job.last_frame) {
        assistant->the_job.last_frame = last_frame || (assistant->max_frames && assistant->max_frames == assistant->current_frame);
    }
    assistant->job_generator->send(assistant->job_generator, write_frame::value, assistant->the_job);
    assistant->the_job.shapes.clear();
}
