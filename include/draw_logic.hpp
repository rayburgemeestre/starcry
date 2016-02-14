#pragma once

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

    void scale(double scale) { scale_ = scale; }
    void width(uint32_t width) { width_ = width; }
    void height(uint32_t height) { height_ = height; }
    void center(double x, double y) { centerX_ = x; centerY_ = y; }
    void offset(double x, double y) { offsetX_ = x; offsetY_ = y; }

private:

    double scale_;
    double centerX_;
    double centerY_;
    double offsetX_;
    double offsetY_;
    uint32_t width_;
    uint32_t height_;
};