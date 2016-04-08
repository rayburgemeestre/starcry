#pragma once

#include <allegro5/allegro_primitives.h>

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
    void render_line(double x1, double y1, double x2, double y2, double size, double r, double g, double b) {
        x1    = ((x1 * scale_) + centerX_) - offsetX_;
        y1    = ((y1 * scale_) + centerY_) - offsetY_;
        x2    = ((x2 * scale_) + centerX_) - offsetX_;
        y2    = ((y2 * scale_) + centerY_) - offsetY_;

        //al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
        //al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
        al_draw_line(x1, y1, x2, y2, al_map_rgb_f(r, g, b), size);

#ifdef NOTDEF
        // need to port this (and perhaps optimize ?)
            double canvasCenterX = m_canvasWidth / 2.0;
            double canvasCenterY = m_canvasHeight / 2.0;

            camera_util::modify_center_xy(canvasCenterX, canvasCenterY, m_scale);

            double posX = (m_scale * m_object->GetX()) + canvasCenterX;
            double posY = (m_scale * m_object->GetY()) + canvasCenterY;
            double posX2 = (m_scale * m_object->GetX2()) + canvasCenterX;
            double posY2 = (m_scale * m_object->GetY2()) + canvasCenterY;
            //double radius = m_scale * m_object->GetRadius();
            double radiusSize = m_scale * m_object->GetRadiusSize();

            Line aline(posX, posY, posX2, posY2, radiusSize);

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
            double aX = 0;
            double aY = 0;
            double bX = 0;
            double bY = 0;
            double cX = 0;
            double cY = 0;
            double dX = 0;
            double dY = 0;

            double angle = get_angle(aline.x, aline.y, aline.x2, aline.y2);

            double radiussizeplusextra = aline.radiussize + 3; /* three extra surrounding pixels so we make up for rounding errors and in a diagonal direction */

            // Get corner a.
            double tmpAngle = angle + 90.0; // go left...
            if (tmpAngle > 360.0)
                tmpAngle -= 360.0;
            double rads = tmpAngle * M_PI / 180;
            aX = aline.x + radiussizeplusextra * cos(rads);
            aY = aline.y + radiussizeplusextra * sin(rads);

            tmpAngle = get_angle(aX, aY, aline.x, aline.y) - 90.0; // go right..
            if (tmpAngle < 0.0)
                tmpAngle += 360.0;
            rads = tmpAngle * M_PI / 180;
            aX = aX + radiussizeplusextra * cos(rads);
            aY = aY + radiussizeplusextra * sin(rads);

            // Get corner d.
            tmpAngle = angle - 90.0;
            if (tmpAngle < 0.0)
                tmpAngle += 360.0;
            rads = tmpAngle * M_PI / 180;
            dX = aline.x + radiussizeplusextra * cos(rads);
            dY = aline.y + radiussizeplusextra * sin(rads);

            tmpAngle = get_angle(dX, dY, aline.x, aline.y) + 90.0;
            if (tmpAngle > 360.0)
                tmpAngle -= 360.0;
            rads = tmpAngle * M_PI / 180;
            dX = dX + radiussizeplusextra * cos(rads);
            dY = dY + radiussizeplusextra * sin(rads);

            // Get corner b.
            tmpAngle = angle + 90.0; // go left...
            if (tmpAngle > 360.0)
                tmpAngle -= 360.0;
            rads = tmpAngle * M_PI / 180;
            bX = aline.x2 + radiussizeplusextra * cos(rads);
            bY = aline.y2 + radiussizeplusextra * sin(rads);

            tmpAngle = get_angle(bX, bY, aline.x2, aline.y2) + 90.0;
            if (tmpAngle > 360.0)
                tmpAngle -= 360.0;
            rads = tmpAngle * M_PI / 180;
            bX = bX + radiussizeplusextra * cos(rads);
            bY = bY + radiussizeplusextra * sin(rads);


            // Get corner c.
            tmpAngle = angle - 90.0;
            if (tmpAngle < 0.0)
                tmpAngle += 360.0;
            rads = tmpAngle * M_PI / 180;
            cX = aline.x2 + radiussizeplusextra * cos(rads);
            cY = aline.y2 + radiussizeplusextra * sin(rads);

            tmpAngle = get_angle(cX, cY, aline.x2, aline.y2) - 90.0;
            if (tmpAngle < 0.0)
                tmpAngle += 360.0;
            rads = tmpAngle * M_PI / 180;
            cX = cX + radiussizeplusextra * cos(rads);
            cY = cY + radiussizeplusextra * sin(rads);

            Line A(aX, aY, aline.x, aline.y, 1);
            Line B(bX, bY, aline.x2, aline.y2, 1);
            Line C(cX, cY, aline.x2, aline.y2, 1);
            Line D(dX, dY, aline.x, aline.y, 1);

#ifdef RENDERER_HIGHQUALITY_RENDER_DEBUG_OUTPUT
            //    aline.Draw(canvas);
    A.Draw(m_pDestCanvas, makecol(255,255,255));
    B.Draw(m_pDestCanvas, makecol(255,255,255));
    C.Draw(m_pDestCanvas, makecol(255,255,255));
    D.Draw(m_pDestCanvas, makecol(255,255,255));
#endif


            Line A2(aX, aY, bX, bY, 1);
            Line B2(bX, bY, cX, cY, 1);
            Line C2(cX, cY, dX, dY, 1);
            Line D2(dX, dY, aX, aY, 1);


            double slopeA = A2.GetSlope();
            double slopeB = B2.GetSlope();
            double slopeC = C2.GetSlope();
            double slopeD = D2.GetSlope();
            double intersectA = A2.GetIntersect();
            double intersectB = B2.GetIntersect();
            double intersectC = C2.GetIntersect();
            double intersectD = D2.GetIntersect();

#ifdef RENDERER_HIGHQUALITY_RENDER_DEBUG_OUTPUT
            A2.Draw(m_pDestCanvas, makecol(255, 0, 0));
    B2.Draw(m_pDestCanvas, makecol(255, 255, 0));
    C2.Draw(m_pDestCanvas, makecol(0, 255, 0));
    D2.Draw(m_pDestCanvas, makecol(0, 255, 255));
#endif RENDERER_HIGHQUALITY_RENDER_DEBUG_OUTPUT


            double topY = aY;
            topY = bY < topY ? bY : topY;
            topY = cY < topY ? cY : topY;
            topY = dY < topY ? dY : topY;

            double bottomY = aY;
            bottomY = bY > bottomY ? bY : bottomY;
            bottomY = cY > bottomY ? cY : bottomY;
            bottomY = dY > bottomY ? dY : bottomY;

            double centerX = ((aline.x - aline.x2) / 2) + aline.x2;
            double centerY = ((aline.y - aline.y2) / 2) + aline.y2;

#ifdef RENDERER_HIGHQUALITY_RENDER_DEBUG_OUTPUT
            circle(m_pDestCanvas, centerX, centerY, 2, makecol(255,255,255));
#endif

            /**
             * Make sure we do not iterate unnecessary pixels,
             *  the ones outside the canvas.
             */
            if (topY < 0) {
                topY = 0;
            }
            if (bottomY > m_canvasHeight) {
                bottomY = m_canvasHeight;
            }

            for (int currentY=topY; currentY<=bottomY; currentY+=1) {
                //circle(canvas, centerX, currentY, 5, makecol(0,255,0));
                //http://www.mathopenref.com/coordintersection.html "When one line is vertical"
                double currentX  = 0;
                double intersectionX1 = 0;
                double intersectionX2 = 0;
                bool hasIntersectionX1 = false;
                bool hasIntersectionX2 = false;
                if (A2.y != A2.y2 && (A2.y < A2.y2 ? A2.y : A2.y2) <= currentY && (A2.y < A2.y2 ? A2.y2 : A2.y) >= currentY) {
                    if (A2.x == A2.x2) { // Todo: change to IsVertical()?
                        // Horizontal line, intersection with an infinite line within
                        //  y-range, cannot have another X then A2.x or A2.x2
                        currentX = A2.x;
                    } else {
                        currentX = (currentY - intersectA) / slopeA;
                    }
                    if (!hasIntersectionX1) {
                        hasIntersectionX1 = true;
                        intersectionX1 = currentX;
                    } else {
                        hasIntersectionX2 = true;
                        intersectionX2 = currentX;
                    }
                    //circlefill(canvas, currentX, currentY, 1, makecol(255,2,2));
                }

                if (B2.y != B2.y2 && (B2.y < B2.y2 ? B2.y : B2.y2) <= currentY && (B2.y < B2.y2 ? B2.y2 : B2.y) >= currentY) {
                    if (B2.x == B2.x2) {
                        // Horizontal line, intersection with an infinite line within
                        //  y-range, cannot have another X then B2.x or B2.x2
                        currentX = B2.x;
                    } else {
                        currentX = (currentY - intersectB) / slopeB;
                    }
                    if (!hasIntersectionX1) {
                        hasIntersectionX1 = true;
                        intersectionX1 = currentX;
                    } else {
                        hasIntersectionX2 = true;
                        intersectionX2 = currentX;
                    }
                }

                if (/*IsFiniteNumber(slopeC) && */C2.y != C2.y2 && (C2.y < C2.y2 ? C2.y : C2.y2) <= currentY && (C2.y < C2.y2 ? C2.y2 : C2.y) >= currentY) {
                    if (C2.x == C2.x2) {
                        currentX = C2.x;
                    } else {
                        currentX = (currentY - intersectC) / slopeC;
                    }
                    if (!hasIntersectionX1) {
                        hasIntersectionX1 = true;
                        intersectionX1 = currentX;
                    } else {
                        hasIntersectionX2 = true;
                        intersectionX2 = currentX;
                    }
                }

                if (/*IsFiniteNumber(slopeD) && */D2.y != D2.y2 && (D2.y < D2.y2 ? D2.y : D2.y2) <= currentY && (D2.y < D2.y2 ? D2.y2 : D2.y) >= currentY) {
                    if (D2.x == D2.x2) {
                        currentX = D2.x;
                    } else {
                        currentX = (currentY - intersectD) / slopeD;
                    }

                    if (!hasIntersectionX1) {
                        hasIntersectionX1 = true;
                        intersectionX1 = currentX;
                    } else {
                        hasIntersectionX2 = true;
                        intersectionX2 = currentX;
                    }
                }

                if (hasIntersectionX1 && hasIntersectionX2) {
                    int xLeft = (intersectionX1 > intersectionX2 ? intersectionX2 : intersectionX1);
                    int xRight = (intersectionX1 > intersectionX2 ? intersectionX1 : intersectionX2);
                    /**
                     * Do not loop through unnecessary pixels
                     */
                    if (xLeft < 0) {
                        xLeft = 0;
                    }
                    if (xRight > m_canvasWidth) {
                        xRight = m_canvasWidth;
                    }
                    for (int x=xLeft; x<=xRight; x++) {
#define SQUARED_DIST(num, num2) ((num - num2) * (num - num2))
                        double distPixel = sqrt(SQUARED_DIST(x, centerX) + SQUARED_DIST(centerY, currentY));
                        double distMax = sqrt(SQUARED_DIST(centerX, aline.x2) + SQUARED_DIST(centerY, aline.y2));

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
                            tmpAngle = angle + 90;
                            if (tmpAngle > 360.0)
                                tmpAngle -= 360.0;
                            rads = tmpAngle * M_PI / 180;
                            double tmpX = x;
                            double tmpY = currentY;
                            tmpX = tmpX + aline.radiussize * cos(rads);
                            tmpY = tmpY + aline.radiussize * sin(rads);

                            tmpAngle = angle + 90 + 180;
                            if (tmpAngle > 360.0)
                                tmpAngle -= 360.0;
                            rads = tmpAngle * M_PI / 180;
                            double tmpX2 = x;
                            double tmpY2 = currentY;
                            tmpX2 = tmpX2 + aline.radiussize * cos(rads);
                            tmpY2 = tmpY2 + aline.radiussize * sin(rads);


                            // It doesn't matter this if this line doesn't cross the center line from all places,
                            //  because we handle it like an infinite line (no begin or ending). we just use slope + intersect.
                            // Edit: nonetheless the line is now extended in such a way it always does..
                            Line tmp(tmpX, tmpY, tmpX2, tmpY2, 1);

                            // I solved the equation by example from http://www.mathopenref.com/coordintersection.html

                            // Cannot calculate intersection if x == x2. No need for it either.
                            // The angle probably was 0.0000001 or something, hence the if-statement
                            // a few lines above wasn't triggered (if angle == 0.0)...
                            // But now after rounding it appears so that x == x2. The small angle
                            // didn't influence a difference between x and x2, that's why this 'extra' check
                            // is required.
                            double nom = (-1 * tmp.GetIntersect()) + aline.GetIntersect();
                            double denom = tmp.GetSlope() - aline.GetSlope();
                            if (aline.x == aline.x2 ||
                                tmp.x == tmp.x2
                                ) {
                                intersectX = tmp.x;
                            } else {
                                intersectX = (nom) / (denom);
                            }
                            // idem..
                            if (tmp.x == tmp.x2) {
                                intersectY = aline.y;
                            } else {
                                intersectY = (intersectX * tmp.GetSlope()) + tmp.GetIntersect();
                            }
                        }

                        if ( (intersectX >= (aline.x < aline.x2 ? aline.x : aline.x2) &&
                              intersectX <= (aline.x < aline.x2 ? aline.x2 : aline.x)) ||

                             (intersectY >= (aline.y < aline.y2 ? aline.y : aline.y2) &&
                              intersectY <= (aline.y < aline.y2 ? aline.y2 : aline.y))/*
                     true*/
                            ) {
                            double distFromCenterline = sqrt(SQUARED_DIST(x, intersectX) + SQUARED_DIST(currentY, intersectY));
                            double testa = (distPixel / distMax);
                            double test = (distFromCenterline / aline.radiussize);
                            RenderLinePixel(x, currentY, testa, test);
                        }
                    }
                }
            }
#endif
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