#pragma once

#include <allegro5/allegro_primitives.h>

static constexpr const auto pi = 3.14159265358979323846;

class coord {
public:
    double x;
    double y;

    coord(double x, double y) : x(x), y(y) {}
};

class line {
public:
    double size;
    double x;
    double y;
    double x2;
    double y2;

public:
    line(double x, double y, double x2, double y2, double size = 1)
        : x(x), y(y), x2(x2), y2(y2), size(size)
    {}

    line(coord a, coord b, double size = 1)
        : x(a.x), y(a.y), x2(b.x), y2(b.y), size(size)
    {}

    coord from() {
        return coord(x, y);
    }
    coord to() {
        return coord(x2, y2);
    }
    bool vertical() const {
        return x == x2;
    }
    bool horizontal() const {
        return y == y2;
    }
    coord center() const {
        return coord(((x - x2) / 2) + x2,
                     ((y - y2) / 2) + y2);
    }

    double slope() const
    {
        return (y - y2) / (x - x2);
    }
    double intersect() const
    {
        // y = mx + b
        double mx = slope() * this->x;
        double y = this->y;
        // see: http://www.purplemath.com/modules/strtlneq.htm
        double b = y - mx;
        return b;
    }
    double angle()
    {
        double dx = x - x2;
        double dy = y - y2;

        if (dx == 0 && dy == 0)
            return 0;

        if (dx == 0) {
            if (dy < 0)
                return 270;
            else
                return 90;
        }

        double slope = dy / dx;
        double angle = atan(slope);
        if (dx < 0)
            angle += pi;

        angle = 180.0 * angle / pi;

        while (angle < 0.0)
            angle += 360.0;

        return angle;
    }

};

inline auto constexpr squared_dist(auto num, auto num2) { return (num - num2) * (num - num2); }

inline coord move_plus(coord c, double angle, double rotate, double move) {
    double tmpAngle = angle + rotate; // go left...
    if (tmpAngle > 360.0)
        tmpAngle -= 360.0;
    double rads = tmpAngle * pi / 180;
    return coord(c.x + move * cos(rads), c.y + move * sin(rads));
}
inline coord move_minus(coord c, double angle, double rotate, double move) {
    double tmpAngle = angle + rotate; // go left...
    if (tmpAngle < 0.0)
        tmpAngle += 360.0;
    double rads = tmpAngle * pi / 180;
    return coord(c.x + move * cos(rads), c.y + move * sin(rads));
}
inline coord move(coord c, double angle, double rotate, double move) {
    if (rotate >= 0) {
        return move_plus(c, angle, rotate, move);
    }
    else {
        return move_minus(c, angle, rotate, move);
    }
}

class draw_logic {
public:

    /**
     * Circle drawing function
     *
     * Optimizations used:
     *
     * - Each pixel for the circle eventually will get a color depending on the distance that pixel has from the
     *   circle center.
     *
     *   - Calculating which pixels to do the calculation for requires computing and to avoid some calculations we
     *     process only a quarter of the circle and derive the other three quarters from it.
     *
     *   - As Circles can be positioned on non-integer coordinates we cannot always re-use the distance calculation
     *     for the three other quarters, as there can be subtle differences.
     *     In case the floating point values contain integers the optimization will be used (reuseSqrt=true).
     *
     * - We will also focus on processing only the pixels visible around it's edges. If the radius is 100 and the
     *   radius size is only 5 pixels, we we only check the pixels between circle radii 97 (inner-) and 103 (outer
     *   circle). For this we calculate the half of the "x chord length" for both the outer- and inner circle, and
     *   substract them.
     */
    template <typename double_type>
    inline void render_circle(double_type circle_x, double_type circle_y, double_type radius, double_type radius_size)
    {
        circle_x             = ((circle_x * scale_) + center_x_) - offset_x_;
        circle_y             = ((circle_y * scale_) + center_y_) - offset_y_;
        radius              *= scale_;
        radius_size         *= scale_;

        bool reuse_sqrt      = floor(circle_x) == circle_x && floor(circle_y) == circle_y;

        // There is a {-1, +1} for compensating rounding that occurs with floating point.
        int radius_outer_circle = round_to_int<double_type>(radius + radius_size + 1);
        int radius_inner_circle = round_to_int<double_type>(radius - radius_size - 1);

        for (int rel_y=0; rel_y<radius_outer_circle; rel_y++) {
            int abs_y_top    = static_cast<int>(circle_y - rel_y);
            int abs_y_bottom = static_cast<int>(circle_y + rel_y);

            if ((abs_y_top < 0) && (abs_y_bottom > static_cast<int>(height_)))
                break;

            int hxcl_outer = half_chord_length<decltype(radius_outer_circle), double_type>(radius_outer_circle, rel_y);
            int hxcl_inner = 0;

            if (radius_inner_circle >= rel_y)
                hxcl_inner = half_chord_length<decltype(radius_inner_circle), double_type>(radius_inner_circle, rel_y);

            for (int rel_x = hxcl_inner; rel_x < hxcl_outer; rel_x++) {
                int abs_x_left  = static_cast<int>(circle_x - rel_x);
                int abs_x_right = static_cast<int>(circle_x + rel_x);

                if (abs_x_left < 0 && abs_x_right > static_cast<int>(width_))
                    continue;

                double_type diff_from_center = reuse_sqrt ? distance<double_type>(abs_x_left, circle_x, abs_y_top, circle_y) : -1;

                render_circle_pixel(radius, radius_size, circle_x, circle_y, abs_x_left, abs_y_top, diff_from_center);

                if (rel_y != 0)
                    render_circle_pixel(radius, radius_size, circle_x, circle_y, abs_x_left, abs_y_bottom, diff_from_center);
                if (rel_x != 0)
                    render_circle_pixel(radius, radius_size, circle_x, circle_y, abs_x_right, abs_y_top, diff_from_center);
                if (rel_x != 0 && rel_y != 0)
                    render_circle_pixel(radius, radius_size, circle_x, circle_y, abs_x_right, abs_y_bottom, diff_from_center);
            }
        }
    }
    template <typename double_type>
    inline void render_text(double_type textX, double_type textY, string text, string align) {
        textX    = ((textX * scale_) + center_x_) - offset_x_;
        textY    = ((textY * scale_) + center_y_) - offset_y_;
        auto alignment = align == "center" ? ALLEGRO_ALIGN_CENTER : ALLEGRO_ALIGN_LEFT;
        al_draw_text(font_, al_map_rgb(255, 255, 255), textX, textY - (14 /*font height*/ / 2), alignment, text.c_str());
    }

    template <typename double_type>
    void render_circle_pixel(double_type radius, double_type radiussize, double_type posX, double_type posY, int absX, int absY, double_type diffFromCenter)
    {
        if (absX < 0 || absY < 0 || absX >= static_cast<int>(width_) || absY >= static_cast<int>(height_))
            return;

        if (diffFromCenter == -1)
            diffFromCenter = distance<double_type>(absX, posX, absY, posY);

        double_type bottom = radius - radiussize;
        double_type colorIdxNormalized = (diffFromCenter - bottom) / (radiussize * 2);
        if (colorIdxNormalized > 1.0 || colorIdxNormalized < 0.0)
            return;

        double_type test = fabs(colorIdxNormalized - 0.5);
        double_type Opacity = test * 2.0;

        //http://liballeg.org/a5docs/trunk/graphics.html#al_load_bitmap_flags
        // by default Allegro pre-multiplies the alpha channel of an image
        // TODO, check al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
        // TODO, check ALLEGRO_NO_PREMULTIPLIED_ALPHA, because I prefer separate alpha channel
        //       unless it's not recommended for other reasons I don't understand..
        Opacity = 1.0 - Opacity;

        al_put_pixel(absX, absY, al_map_rgba_f(0, 0, (1.0 /*- test*/) * Opacity, 0));
    }

    template <typename double_type>
    void render_line(double x1, double y1, double x2, double y2, double size, double red, double green, double blue) {
        x1    = ((x1 * scale_) + center_x_) - offset_x_;
        y1    = ((y1 * scale_) + center_y_) - offset_y_;
        x2    = ((x2 * scale_) + center_x_) - offset_x_;
        y2    = ((y2 * scale_) + center_y_) - offset_y_;
        size  = size * scale_;

        line line_(x1, y1, x2, y2, size);

        /**
         * Calculate corners a, b, c, d for line.
         * Then construct lines A, B, C, D to create a square.
         *   a          A          b
         *    +-------------------+
         * D  |                   |   B
         *    +-------------------+
         *   d          C          c
         */
        // three extra surrounding pixels so we make up for rounding errors and in a diagonal direction
        double radiussizeplusextra = line_.size + 3;
        coord a = move(line_.from(), line_.angle(), 90, radiussizeplusextra);
        coord d = move(line_.from(), line_.angle(), -90, radiussizeplusextra);
        coord b = move(line_.to(), line_.angle(), 90, radiussizeplusextra);
        coord c = move(line_.to(), line_.angle(), -90, radiussizeplusextra);
        a = move(a, line(a, line_.from()).angle(), -90, radiussizeplusextra);
        d = move(d, line(d, line_.from()).angle(), 90, radiussizeplusextra);
        b = move(b, line(b, line_.to()).angle(), 90, radiussizeplusextra);
        c = move(c, line(c, line_.to()).angle(), 90, radiussizeplusextra);

        line A(a, b), B(b, c), C(c, d), D(d, a);

        double top_y = std::min(std::min(std::min(a.y, b.y), c.y), d.y);
        double bottom_y = std::min(std::min(std::max(a.y, b.y), c.y), d.y);

        // Make sure we do not iterate unnecessary pixels, the ones outside the canvas.
        if (top_y < 0)
            top_y = 0;
        if (bottom_y > height_)
            bottom_y = height_;

        for (int current_y=top_y; current_y<=bottom_y; current_y+=1) {
            //http://www.mathopenref.com/coordintersection.html "When one line is vertical"
            double current_x  = 0;
            double intersection_x1 = 0;
            double intersection_x2 = 0;
            bool has_intersection_x1 = false;
            bool has_intersection_x2 = false;
            auto check_intersection = [&](const auto &line2) {
                if (  line2.y != line2.y2 && (line2.y < line2.y2 ? line2.y : line2.y2) <= current_y && (line2.y < line2.y2 ? line2.y2 : line2.y) >= current_y) {
                //if (!line2.horizontal() && min(line2.y, line2.y2) <= current_y && min(line2.y, line2.y2) >= current_y) {
                    if (line2.vertical()) {
                        // Horizontal line, intersection with an infinite line within
                        //  y-range, cannot have another X then A2.x or A2.x2
                        current_x = line2.x;
                    } else {
                        current_x = (current_y - line2.intersect()) / line2.slope();
                    }
                    if (!has_intersection_x1) {
                        has_intersection_x1 = true;
                        intersection_x1 = current_x;
                    } else {
                        has_intersection_x2 = true;
                        intersection_x2 = current_x;
                    }
                }
            };
            check_intersection(A);
            check_intersection(B);
            check_intersection(C);
            check_intersection(D);

            if (has_intersection_x1 && has_intersection_x2) {
                int x_left = min(intersection_x1, intersection_x2);
                int x_right = max(intersection_x1, intersection_x2);
                // Do not loop through unnecessary pixels
                if (x_left < 0)
                    x_left = 0;
                if (x_right > width_)
                    x_right = width_;

                for (int x=x_left; x<=x_right; x++) {
                    double distPixel = sqrt(squared_dist(x, line_.center().x) + squared_dist(line_.center().y, current_y));
                    double distMax = sqrt(squared_dist(line_.center().x, line_.x2) + squared_dist(line_.center().y, line_.y2));
                    double intersect_x = 0;
                    double intersect_y = 0;
                    // These if-statements probably need some documentation
                    // TODO: not 100% sure if this is correct, as in, doesn't it 'flip' the line (horizontally or vertically)
                    //       also, does it matter?
                    if (line_.angle() == 180|| line_.angle() == 0 || line_.angle() == 360) {
                        al_draw_line(x1, y1, x2, y2, al_map_rgb_f(1, 0, 0), size);
                        return;
                        intersect_x = x;
                        intersect_y = line_.y;
                    }
                    else if (line_.angle() == 270 || line_.angle() ==90) {
                        al_draw_line(x1, y1, x2, y2, al_map_rgb_f(0, 1, 0), size);
                        return;
                        intersect_x = line_.center().x;
                        intersect_y = current_y;
                    }
                    else {
                        al_draw_line(x1, y1, x2, y2, al_map_rgb_f(0, 0, 1), size);
                        return;
                        coord tmp1 = move_plus(coord(x, current_y), line_.angle(), 90, line_.size);
                        coord tmp2 = move_plus(coord(x, current_y), line_.angle(), 90 + 180, line_.size);

                        // It doesn't matter this if this line doesn't cross the center line from all places,
                        //  because we handle it like an infinite line (no begin or ending). we just use slope + intersect.
                        // Edit: nonetheless the line is now extended in such a way it always does..
                        line tmp(tmp1, tmp2);

                        // I solved the equation by example from http://www.mathopenref.com/coordintersection.html

                        // Cannot calculate intersection if x == x2. No need for it either.
                        // The angle probably was 0.0000001 or something, hence the if-statement
                        // a few lines above wasn't triggered (if angle == 0.0)...
                        // But now after rounding it appears so that x == x2. The small angle
                        // didn't influence a difference between x and x2, that's why this 'extra' check
                        // is required.
                        double nom = (-1 * tmp.intersect()) + line_.intersect();
                        double denom = tmp.slope() - line_.slope();
                        if (line_.vertical() || tmp.vertical()) {
                            intersect_x = tmp.x;
                        } else {
                            intersect_x = (nom) / (denom);
                        }

                        if (tmp.vertical()) {
                            intersect_y = line_.y;
                        } else {
                            intersect_y = (intersect_x * tmp.slope()) + tmp.intersect();
                        }
                    }

                    if ( (intersect_x >= min(line_.x, line_.x2) && intersect_x <= max(line_.x, line_.x2)) ||
                         (intersect_y >= min(line_.y, line_.y2) && intersect_y <= max(line_.y, line_.y2))
                    ) {
                        double dist_from_center_line = sqrt(squared_dist(x, intersect_x) + squared_dist(current_y, intersect_y));
                        double normalized_dist_from_center = (distPixel / distMax);
                        double normalized_dist_from_line = (dist_from_center_line / line_.size);
                        render_line_pixel<double_type>(x, current_y, normalized_dist_from_center, normalized_dist_from_line);
                    }
                }
            }
        }

        //al_draw_line(x1, y1, x2, y2, al_map_rgb_f(red, green, blue), size);
        //if (red > 0.2 || green > 0.2 || blue > 0.2)
        //    al_draw_line(x1, y1, x2, y2, al_map_rgb_f(red, green, blue), 2.0);
    }

    template <typename double_type>
    void render_line_pixel(int absX, int absY, double normalizedDistFromCenter, double normalizedDistFromLine) {
        if (normalizedDistFromCenter > 1.0 ||
            normalizedDistFromCenter < 0.0) {
            return;
        }
        if (normalizedDistFromLine > 1.0 ||
            normalizedDistFromLine < 0.0) {
            return;
        }

        //CompiledGradient *ptrGradient = m_object->GetGradient();
        //LColor color = ptrGradient->getColor( (1.0 - normalizedDistFromLine));
        double num = 1.0 - (1.0 - normalizedDistFromCenter) * (1.0 - normalizedDistFromLine);
        //num = 1.0 - num; // just to get it to look like it supposed to a bit right now
        //LColor color = ptrGradient->getColor( num );

        //al_put_pixel(absX, absY, al_map_rgba_f(0, 0, (1.0 /*- test*/) * num, 0));
        auto bg = al_get_pixel(al_get_target_bitmap(), absX, absY);
        bg.b = (bg.b * num) + 1.0 * (1.0 - num); // we blend ourselves..
        //al_put_pixel(absX, absY, al_map_rgba_f(bg.r, bg.g, bg.b, 0));
        al_put_pixel(absX, absY, al_map_rgba_f(1, 0, 0, 0));
    }

    void scale(double scale) { scale_ = scale; }
    void width(uint32_t width) { width_ = width; }
    void height(uint32_t height) { height_ = height; }
    void center(double x, double y) { center_x_ = x; center_y_ = y; }
    void offset(double x, double y) { offset_x_ = x; offset_y_ = y; }
    void font(ALLEGRO_FONT *font) { font_ = font; }

private:

    double scale_;
    double center_x_;
    double center_y_;
    double offset_x_;
    double offset_y_;
    uint32_t width_;
    uint32_t height_;
    ALLEGRO_FONT * font_;
};
