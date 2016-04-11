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

inline coord move_plus(double x, double y, double angle, double rotate, double move) {
    double tmpAngle = angle + rotate; // go left...
    if (tmpAngle > 360.0)
        tmpAngle -= 360.0;
    double rads = tmpAngle * pi / 180;
    return coord(x + move * cos(rads), y + move * sin(rads));
}
inline coord move_minus(double x, double y, double angle, double rotate, double move) {
    double tmpAngle = angle + rotate; // go left...
    if (tmpAngle < 0.0)
        tmpAngle += 360.0;
    double rads = tmpAngle * pi / 180;
    return coord(x + move * cos(rads), y + move * sin(rads));
}
inline coord move(double x, double y, double angle, double rotate, double move) {
    if (rotate >= 0) {
        return move_plus(x, y, angle, rotate, move);
    }
    else {
        return move_minus(x, y, angle, rotate, move);
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
    inline void render_circle(double_type circleX, double_type circleY, double_type radius, double_type radiusSize)
    {
        //al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
        //al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

        circleX             = ((circleX * scale_) + centerX_) - offsetX_;
        circleY             = ((circleY * scale_) + centerY_) - offsetY_;
        radius             *= scale_;
        radiusSize         *= scale_;

        bool reuseSqrt      = floor(circleX) == circleX && floor(circleY) == circleY;

        // There is a {-1, +1} for compensating rounding that occurs with floating point.
        int radiusOuterCircle = round_to_int<double_type>(radius + radiusSize + 1);
        int radiusInnerCircle = round_to_int<double_type>(radius - radiusSize - 1);

        for (int relY=0; relY<radiusOuterCircle; relY++) {
            int absYTop    = static_cast<int>(circleY - relY);
            int absYBottom = static_cast<int>(circleY + relY);

            if ((absYTop < 0) && (absYBottom > static_cast<int>(height_)))
                break;

            int hxcl_outer = half_chord_length<decltype(radiusOuterCircle), double_type>(radiusOuterCircle, relY);
            int hxcl_inner = 0;

            if (radiusInnerCircle >= relY)
                hxcl_inner = half_chord_length<decltype(radiusInnerCircle), double_type>(radiusInnerCircle, relY);

            for (int relX = hxcl_inner; relX < hxcl_outer; relX++) {
                int absXLeft  = static_cast<int>(circleX - relX);
                int absXRight = static_cast<int>(circleX + relX);

                if (absXLeft < 0 && absXRight > static_cast<int>(width_))
                    continue;

                double_type diffFromCenter = reuseSqrt ? distance<double_type>(absXLeft, circleX, absYTop, circleY) : -1;

                render_circle_pixel(radius, radiusSize, circleX, circleY, absXLeft, absYTop, diffFromCenter);

                if (relY != 0)
                    render_circle_pixel(radius, radiusSize, circleX, circleY, absXLeft, absYBottom, diffFromCenter);
                if (relX != 0)
                    render_circle_pixel(radius, radiusSize, circleX, circleY, absXRight, absYTop, diffFromCenter);
                if (relX != 0 && relY != 0)
                    render_circle_pixel(radius, radiusSize, circleX, circleY, absXRight, absYBottom, diffFromCenter);
            }
        }
    }
    template <typename double_type>
    inline void render_text(double_type textX, double_type textY, string text, string align) {
        textX    = ((textX * scale_) + centerX_) - offsetX_;
        textY    = ((textY * scale_) + centerY_) - offsetY_;
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
        x1    = ((x1 * scale_) + centerX_) - offsetX_;
        y1    = ((y1 * scale_) + centerY_) - offsetY_;
        x2    = ((x2 * scale_) + centerX_) - offsetX_;
        y2    = ((y2 * scale_) + centerY_) - offsetY_;
        size  = size * scale_;

        //al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
        //al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
        //al_draw_line(x1, y1, x2, y2, al_map_rgb_f(red, green, blue), size);

        line aline(x1, y1, x2, y2, size);

        /**
         * Create rectangle for aline.
         *
         * With corners:
         *   a                     b
         * x  +-------------------+  x2
         * y  +-------------------+  y2
         *   d                     c
         *
         * As lines:
         *              A
         *  D +-------------------+ B
         *    +-------------------+
         *              C
         */
        double angle = aline.angle();

        // three extra surrounding pixels so we make up for rounding errors and in a diagonal direction
        double radiussizeplusextra = aline.size + 3;

        coord a = move(aline.x, aline.y, aline.angle(), 90, radiussizeplusextra);
        coord d = move(aline.x, aline.y, aline.angle(), -90, radiussizeplusextra);
        coord b = move(aline.x2, aline.y2, aline.angle(), 90, radiussizeplusextra);
        coord c = move(aline.x2, aline.y2, aline.angle(), -90, radiussizeplusextra);

        a = move(a.x, a.y, line(a.x, a.y, aline.x, aline.y, 1).angle(), -90, radiussizeplusextra);
        d = move(d.x, d.y, line(d.x, d.y, aline.x, aline.y, 1).angle(), 90, radiussizeplusextra);
        b = move(b.x, b.y, line(b.x, b.y, aline.x2, aline.y2, 1).angle(), 90, radiussizeplusextra);
        c = move(c.x, c.y, line(c.x, c.y, aline.x2, aline.y2, 1).angle(), 90, radiussizeplusextra);

        line A(a, b);
        line B(b, c);
        line C(c, d);
        line D(d, a);

        double topY = std::min({a.y, b.y, c.y, d.y});
        double bottomY = std::max({a.y, b.y, c.y, d.y});

        double centerX = ((aline.x - aline.x2) / 2) + aline.x2;
        double centerY = ((aline.y - aline.y2) / 2) + aline.y2;

        // Make sure we do not iterate unnecessary pixels, the ones outside the canvas.
        if (topY < 0)
            topY = 0;
        if (bottomY > height_)
            bottomY = height_;

        for (int currentY=topY; currentY<=bottomY; currentY+=1) {
            //circle(canvas, centerX, currentY, 5, makecol(0,255,0));
            //http://www.mathopenref.com/coordintersection.html "When one line is vertical"
            double currentX  = 0;
            double intersectionX1 = 0;
            double intersectionX2 = 0;
            bool hasIntersectionX1 = false;
            bool hasIntersectionX2 = false;

            auto lmb = [&](const auto &A2) {
                if (A2.y != A2.y2 && (A2.y < A2.y2 ? A2.y : A2.y2) <= currentY && (A2.y < A2.y2 ? A2.y2 : A2.y) >= currentY) {
                    if (A2.x == A2.x2) { // Todo: change to IsVertical()?
                        // Horizontal line, intersection with an infinite line within
                        //  y-range, cannot have another X then A2.x or A2.x2
                        currentX = A2.x;
                    } else {
                        currentX = (currentY - A2.intersect()) / A2.slope();
                    }
                    if (!hasIntersectionX1) {
                        hasIntersectionX1 = true;
                        intersectionX1 = currentX;
                    } else {
                        hasIntersectionX2 = true;
                        intersectionX2 = currentX;
                    }
                }
            };
            lmb(A);
            lmb(B);
            lmb(C);
            lmb(D);

            if (hasIntersectionX1 && hasIntersectionX2) {
                int xLeft = min(intersectionX1, intersectionX2);
                int xRight = max(intersectionX1, intersectionX2);
                // Do not loop through unnecessary pixels
                if (xLeft < 0)
                    xLeft = 0;
                if (xRight > width_)
                    xRight = width_;

                for (int x=xLeft; x<=xRight; x++) {
                    double distPixel = sqrt(squared_dist(x, centerX) + squared_dist(centerY, currentY));
                    double distMax = sqrt(squared_dist(centerX, aline.x2) + squared_dist(centerY, aline.y2));

                    double intersectX = 0;
                    double intersectY = 0;
                    // These if-statements probably need some documentation
                    // EDIT: not sure if this is correct, as in, doesn't it 'flip' the line (horizontally or vertically??)
                    if (angle == 180|| angle == 0 || angle == 360) {
                        intersectX = x;
                        intersectY = aline.y;
                    }
                    else if (angle == 270 || angle==90) {
                        intersectX = centerX;
                        intersectY = currentY;
                    }
                    else {
                        coord tmp1 = move_plus(x, currentY, angle, 90, aline.size);
                        coord tmp2 = move_plus(x, currentY, angle, 90 + 180, aline.size);

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
                        double nom = (-1 * tmp.intersect()) + aline.intersect();
                        double denom = tmp.slope() - aline.slope();
                        if (aline.x == aline.x2 || tmp.x == tmp.x2) {
                            intersectX = tmp.x;
                        } else {
                            intersectX = (nom) / (denom);
                        }
                        // idem..
                        if (tmp.x == tmp.x2) {
                            intersectY = aline.y;
                        } else {
                            intersectY = (intersectX * tmp.slope()) + tmp.intersect();
                        }
                    }

                    if ( (intersectX >= min(aline.x, aline.x2) && intersectX <= max(aline.x, aline.x2)) ||
                         (intersectY >= min(aline.y, aline.y2) && intersectY <= max(aline.y, aline.y2))
                    ) {
                        double distFromCenterline = sqrt(squared_dist(x, intersectX) + squared_dist(currentY, intersectY));
                        double testa = (distPixel / distMax);
                        double test = (distFromCenterline / aline.size);
                        render_line_pixel<double_type>(x, currentY, testa, test);
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
        al_put_pixel(absX, absY, al_map_rgba_f(bg.r, bg.g, bg.b, 0));
    }

    void scale(double scale) { scale_ = scale; }
    void width(uint32_t width) { width_ = width; }
    void height(uint32_t height) { height_ = height; }
    void center(double x, double y) { centerX_ = x; centerY_ = y; }
    void offset(double x, double y) { offsetX_ = x; offsetY_ = y; }
    void font(ALLEGRO_FONT *font) { font_ = font; }

private:

    double scale_;
    double centerX_;
    double centerY_;
    double offsetX_;
    double offsetY_;
    uint32_t width_;
    uint32_t height_;
    ALLEGRO_FONT * font_;
};
