/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/**
 * All the following macros come from Nathan Moinvaziri.
 * http://www.nathanm.com/photoshop-blending-math/
 */

typedef double float64;
typedef int int32;
typedef unsigned char uint8;

#define ChannelBlend_Normal(B,L)     ((L))
#define ChannelBlend_Lighten(B,L)    (((L > B) ? L:B))
#define ChannelBlend_Darken(B,L)     (L > B ? B : L)
#define ChannelBlend_Multiply(B,L)   (((B * L) / 1.0))
#define ChannelBlend_Average(B,L)    (((B + L) / 2))
#define ChannelBlend_Add(B,L)        ((std::min(1.0, (B + L))))
#define ChannelBlend_Subtract(B,L)   (((B + L < 1.0) ? 0:(B + L - 1.0)))
#define ChannelBlend_Difference(B,L) ((abs(B - L)))
#define ChannelBlend_Negation(B,L)   ((1.0 - abs(1.0 - B - L)))
#define ChannelBlend_Screen(B,L)     ((1.0 - (((1.0 - B) * ((1.0 - L)) / (std::pow(2, 8) / 255)))))
#define ChannelBlend_Exclusion(B,L)  ((B + L - 2 * B * L / 1.0))
#define ChannelBlend_Overlay(B,L)    (((L < 0.5) ? (2 * B * L / 1.0):(1.0 - 2 * (1.0 - B) * (1.0 - L) / 1.0)))
#define ChannelBlend_SoftLight(B,L)  (((L < 0.5)?(2*((B/ (std::pow(2, 1) / 255))+0.25))*((float)L/1.0):(1.0-(2*(1.0-((B / (std::pow(2, 1)/255))+0.25))*(float)(1.0-L)/1.0))))
#define ChannelBlend_HardLight(B,L)  (ChannelBlend_Overlay(L,B))
#define ChannelBlend_ColorDodge(B,L) (((L == 1.0) ? L:std::min(1.0, ((B * (std::pow(2, 8)/255) ) / (1.0 - L)))))
#define ChannelBlend_ColorBurn(B,L)  (((L == 0) ? L:std::max(0.0, (1.0 - ((1.0 - B) * (std::pow(2, 8)/255) ) / L))))
#define ChannelBlend_LinearDodge(B,L)(ChannelBlend_Add(B,L))
#define ChannelBlend_LinearBurn(B,L) (ChannelBlend_Subtract(B,L))
#define ChannelBlend_LinearLight(B,L)((L < 0.5)?ChannelBlend_LinearBurn(B,(2 * L)):ChannelBlend_LinearDodge(B,(2 * (L - 0.5))))
#define ChannelBlend_VividLight(B,L) ((L < 0.5)?ChannelBlend_ColorBurn(B,(2 * L)):ChannelBlend_ColorDodge(B,(2 * (L - 0.5))))
#define ChannelBlend_PinLight(B,L)   ((L < 0.5)?ChannelBlend_Darken(B,(2 * L)):ChannelBlend_Lighten(B,(2 * (L - 0.5))))
#define ChannelBlend_HardMix(B,L)    (((ChannelBlend_VividLight(B,L) < 0.5) ? 0:1.0))
#define ChannelBlend_Reflect(B,L)    (((L == 1.0) ? L:std::min(1.0, (B * B / (1.0 - L)))))
#define ChannelBlend_Glow(B,L)       (ChannelBlend_Reflect(L,B))
#define ChannelBlend_Phoenix(B,L)    ((std::min(B,L) - std::max(B,L) + 1.0))
#define ChannelBlend_Alpha(B,L,O)    ((O * B + (1 - O) * L))
#define ChannelBlend_AlphaF(B,L,F,O) (ChannelBlend_Alpha(F(B,L),B,O))

#define ColorBlend_Buffer(T,B,L,M)      (T)[0] = ChannelBlend_##M((B)[0], (L)[0]), \
                                        (T)[1] = ChannelBlend_##M((B)[1], (L)[1]), \
                                        (T)[2] = ChannelBlend_##M((B)[2], (L)[2])

#define ColorBlend_Normal(T,B,L)        (ColorBlend_Buffer(T,B,L,Normal))
#define ColorBlend_Lighten(T,B,L)       (ColorBlend_Buffer(T,B,L,Lighten))
#define ColorBlend_Darken(T,B,L)        (ColorBlend_Buffer(T,B,L,Darken))
#define ColorBlend_Multiply(T,B,L)      (ColorBlend_Buffer(T,B,L,Multiply))
#define ColorBlend_Average(T,B,L)       (ColorBlend_Buffer(T,B,L,Average))
#define ColorBlend_Add(T,B,L)           (ColorBlend_Buffer(T,B,L,Add))
#define ColorBlend_Subtract(T,B,L)      (ColorBlend_Buffer(T,B,L,Subtract))
#define ColorBlend_Difference(T,B,L)    (ColorBlend_Buffer(T,B,L,Difference))
#define ColorBlend_Negation(T,B,L)      (ColorBlend_Buffer(T,B,L,Negation))
#define ColorBlend_Screen(T,B,L)        (ColorBlend_Buffer(T,B,L,Screen))
#define ColorBlend_Exclusion(T,B,L)     (ColorBlend_Buffer(T,B,L,Exclusion))
#define ColorBlend_Overlay(T,B,L)       (ColorBlend_Buffer(T,B,L,Overlay))
#define ColorBlend_SoftLight(T,B,L)     (ColorBlend_Buffer(T,B,L,SoftLight))
#define ColorBlend_HardLight(T,B,L)     (ColorBlend_Buffer(T,B,L,HardLight))
#define ColorBlend_ColorDodge(T,B,L)    (ColorBlend_Buffer(T,B,L,ColorDodge))
#define ColorBlend_ColorBurn(T,B,L)     (ColorBlend_Buffer(T,B,L,ColorBurn))
#define ColorBlend_LinearDodge(T,B,L)   (ColorBlend_Buffer(T,B,L,LinearDodge))
#define ColorBlend_LinearBurn(T,B,L)    (ColorBlend_Buffer(T,B,L,LinearBurn))
#define ColorBlend_LinearLight(T,B,L)   (ColorBlend_Buffer(T,B,L,LinearLight))
#define ColorBlend_VividLight(T,B,L)    (ColorBlend_Buffer(T,B,L,VividLight))
#define ColorBlend_PinLight(T,B,L)      (ColorBlend_Buffer(T,B,L,PinLight))
#define ColorBlend_HardMix(T,B,L)       (ColorBlend_Buffer(T,B,L,HardMix))
#define ColorBlend_Reflect(T,B,L)       (ColorBlend_Buffer(T,B,L,Reflect))
#define ColorBlend_Glow(T,B,L)          (ColorBlend_Buffer(T,B,L,Glow))
#define ColorBlend_Phoenix(T,B,L)       (ColorBlend_Buffer(T,B,L,Phoenix))
#define ColorBlend_Hue(T,B,L)            ColorBlend_Hls(T,B,L,HueL,LuminationB,SaturationB)
#define ColorBlend_Saturation(T,B,L)     ColorBlend_Hls(T,B,L,HueB,LuminationB,SaturationL)
#define ColorBlend_Color(T,B,L)          ColorBlend_Hls(T,B,L,HueL,LuminationB,SaturationL)
#define ColorBlend_Luminosity(T,B,L)     ColorBlend_Hls(T,B,L,HueB,LuminationL,SaturationB)

#define ColorBlend_Hls(T,B,L,O1,O2,O3) {                                        \
    float64 HueB, LuminationB, SaturationB;                                     \
    float64 HueL, LuminationL, SaturationL;                                     \
    Color_RgbToHls((B)[2],(B)[1],(B)[0], &HueB, &LuminationB, &SaturationB);    \
    Color_RgbToHls((L)[2],(L)[1],(L)[0], &HueL, &LuminationL, &SaturationL);    \
    Color_HlsToRgb(O1,O2,O3,&(T)[2],&(T)[1],&(T)[0]);                           \
    }

inline int32 Color_HueToRgb(float64 M1, float64 M2, float64 Hue, float64 *Channel)
{
    if (Hue < 0.0)
        Hue += 1.0;
    else if (Hue > 1.0)
        Hue -= 1.0;

    if ((6.0 * Hue) < 1.0)
        *Channel = (M1 + (M2 - M1) * Hue * 6.0);
    else if ((2.0 * Hue) < 1.0)
        *Channel = (M2);
    else if ((3.0 * Hue) < 2.0)
        *Channel = (M1 + (M2 - M1) * ((2.0F / 3.0F) - Hue) * 6.0);
    else
        *Channel = (M1);

    return true;
}

inline int32 Color_RgbToHls(float Redf, float Greenf, float Bluef, float64 *Hue, float64 *Lumination, float64 *Saturation)
{
    float64 Delta;
    float64 Max, Min;

    Max     = std::max(std::max(Redf, Greenf), Bluef);
    Min     = std::min(std::min(Redf, Greenf), Bluef);

    *Hue        = 0;
    *Lumination = (Max + Min) / 2.0F;
    *Saturation = 0;

    if (Max == Min)
        return true;

    Delta = (Max - Min);

    if (*Lumination < 0.5)
        *Saturation = Delta / (Max + Min);
    else
        *Saturation = Delta / (2.0 - Max - Min);

    if (Redf == Max)
        *Hue = (Greenf - Bluef) / Delta;
    else if (Greenf == Max)
        *Hue = 2.0 + (Bluef - Redf) / Delta;
    else
        *Hue = 4.0 + (Redf - Greenf) / Delta;

    *Hue /= 6.0;

    if (*Hue < 0.0)
        *Hue += 1.0;

    return true;
}

inline int32 Color_HlsToRgb(float64 Hue, float64 Lumination, float64 Saturation, float *Red, float *Green, float *Blue)
{
    float64 M1, M2;
    float64 Redf, Greenf, Bluef;

    if (Saturation == 0)
    {
        Redf    = Lumination;
        Greenf  = Lumination;
        Bluef   = Lumination;
    }
    else
    {
        if (Lumination <= 0.5)
            M2 = Lumination * (1.0 + Saturation);
        else
            M2 = Lumination + Saturation - Lumination * Saturation;

        M1 = (2.0 * Lumination - M2);

        Color_HueToRgb(M1, M2, Hue + (1.0F / 3.0F), &Redf);
        Color_HueToRgb(M1, M2, Hue, &Greenf);
        Color_HueToRgb(M1, M2, Hue - (1.0F / 3.0F), &Bluef);
    }

    *Red    = (float)Redf;
    *Blue   = (float)Bluef;
    *Green  = (float)Greenf;

    return true;
}

//#define COLOR_OPAQUE                (0)
//#define COLOR_TRANSPARENT           (127)
//
//#define RGB_SIZE                    (3)
//#define RGB_BPP                     (24)
//#define RGB_MAXRED                  (255)
//#define RGB_MAXGREEN                (255)
//#define RGB_MAXBLUE                 (255)
//
//#define ARGB_SIZE                   (4)
//#define ARGB_BPP                    (32)
//#define ARGB_MAXALPHA               (127)
//#define ARGB_MAXRED                 (RGB_MAXRED)
//#define ARGB_MAXGREEN               (RGB_MAXGREEN)
//#define ARGB_MAXBLUE                (RGB_MAXBLUE)

/*********************************************************************/

struct normal
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Normal(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Normal(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Normal(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct lighten
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Lighten(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Lighten(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Lighten(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct darken
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Darken(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Darken(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Darken(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct multiply
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Multiply(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Multiply(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Multiply(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct average
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Average(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Average(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Average(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct add
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Add(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Add(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Add(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct subtract
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Subtract(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Subtract(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Subtract(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct difference
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Difference(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Difference(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Difference(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct negation_
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Negation(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Negation(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Negation(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct screen
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Screen(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Screen(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Screen(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct exclusion
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Exclusion(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Exclusion(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Exclusion(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct overlay
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Overlay(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Overlay(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Overlay(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct softlight
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_SoftLight(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_SoftLight(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_SoftLight(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct hardlight
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_HardLight(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_HardLight(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_HardLight(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct colordodge
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_ColorDodge(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_ColorDodge(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_ColorDodge(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct colorburn
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_ColorBurn(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_ColorBurn(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_ColorBurn(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct lineardodge
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_LinearDodge(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_LinearDodge(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_LinearDodge(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct linearburn
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_LinearBurn(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_LinearBurn(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_LinearBurn(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct linearlight
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_LinearLight(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_LinearLight(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_LinearLight(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct vividlight
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_VividLight(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_VividLight(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_VividLight(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct pinlight
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_PinLight(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_PinLight(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_PinLight(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct hardmix
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_HardMix(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_HardMix(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_HardMix(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct reflect
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Reflect(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Reflect(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Reflect(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct glow
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Glow(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Glow(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Glow(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};
struct phoenix
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        return color(
            ChannelBlend_Phoenix(basecolor.get_r(), blendcolor.get_r()),
            ChannelBlend_Phoenix(basecolor.get_g(), blendcolor.get_g()),
            ChannelBlend_Phoenix(basecolor.get_b(), blendcolor.get_b()),
            blendcolor.get_a()
        );
    }
};

////////////////////////////////////////////////////////////////////
// The following three blending modes I will refactor in the future
////////////////////////////////////////////////////////////////////

struct hue
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        float64 HueB, LuminationB, SaturationB;
        float64 HueL, LuminationL, SaturationL;
        Color_RgbToHls(basecolor.get_r(),basecolor.get_g(),basecolor.get_b(), &HueB, &LuminationB, &SaturationB);
        Color_RgbToHls(blendcolor.get_r(), blendcolor.get_g(), blendcolor.get_b(), &HueL, &LuminationL, &SaturationL);
        float r, g, b;
        Color_HlsToRgb(HueL,LuminationB,SaturationB, &r, &g, &b);
        return color(r, g, b, blendcolor.get_a());
    }
};
struct saturation
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        float64 HueB, LuminationB, SaturationB;
        float64 HueL, LuminationL, SaturationL;
        Color_RgbToHls(basecolor.get_r(),basecolor.get_g(),basecolor.get_b(), &HueB, &LuminationB, &SaturationB);
        Color_RgbToHls(blendcolor.get_r(), blendcolor.get_g(), blendcolor.get_b(), &HueL, &LuminationL, &SaturationL);
        float r, g, b;
        Color_HlsToRgb(HueB,LuminationB,SaturationL, &r, &g, &b);
        return color(r, g, b, blendcolor.get_a());
    }
};
struct color_blend
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        float64 HueB, LuminationB, SaturationB;
        float64 HueL, LuminationL, SaturationL;
        Color_RgbToHls(basecolor.get_r(),basecolor.get_g(),basecolor.get_b(), &HueB, &LuminationB, &SaturationB);
        Color_RgbToHls(blendcolor.get_r(), blendcolor.get_g(), blendcolor.get_b(), &HueL, &LuminationL, &SaturationL);
        float r, g, b;
        Color_HlsToRgb(HueL,LuminationB,SaturationL, &r, &g, &b);
        return color(r, g, b, blendcolor.get_a());
    }
};
struct luminosity
{
    inline const color blend(const color &basecolor, const color &blendcolor) const {
        float64 HueB, LuminationB, SaturationB;
        float64 HueL, LuminationL, SaturationL;
        Color_RgbToHls(basecolor.get_r(),basecolor.get_g(),basecolor.get_b(), &HueB, &LuminationB, &SaturationB);
        Color_RgbToHls(blendcolor.get_r(), blendcolor.get_g(), blendcolor.get_b(), &HueL, &LuminationL, &SaturationL);
        float r, g, b;
        Color_HlsToRgb(HueB,LuminationL,SaturationB, &r, &g, &b);
        return color(r, g, b, blendcolor.get_a());
    }
};

template <typename blend_type>
const color blender(const color &color1, const color &color2)
{
    blend_type t1;
    return t1.blend(color1, color2);
}

