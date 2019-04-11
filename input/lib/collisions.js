/*
 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

function circles_collide(a, b)
{
    const radius_sum = a.radius + b.radius + a.radius_size + b.radius_size;
    const a_center = a.as_vec2d();
    const b_center = b.as_vec2d();
    const distance = subtract_vector(a_center, b_center);
    return dot_product(distance, distance) <= radius_sum * radius_sum;
}
