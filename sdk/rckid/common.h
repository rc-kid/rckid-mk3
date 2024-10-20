#pragma once

#include <platform.h>
#include <platform/color_strip.h>
#include "backend_config.h"

// Forward declaration for the RCKid firmware class so that we can access status internals
class RCKid;
namespace rckid {

    /** RGB LED effects 
     
        - Solid - display color (color, timer?)
        - Fadedisplay color, vary brightness (color, speed, min, max)
        - display color, vary hue (hue, speed)
     
     */
    PACKED(class RGBEffect {
    public:
        enum class Kind : uint8_t {
            Off,
            Solid, 
            Breathe,
            Rainbow,
        }; 

        PACKED(struct Rainbow {
            uint8_t hue;
            uint8_t step; 
            uint8_t brightness;
        }); 

        /** Kind of the effect. 
         */
        Kind kind;
        /** Effect transition speed. 
         */
        uint8_t speed;
        /** Duration of the effect in seconds. 
         */
        uint8_t duration;

        PACKED(union {
            platform::Color color;
            Rainbow rainbow;
        });

        RGBEffect(): kind{Kind::Off}, speed{1}, duration{0}, color{platform::Color::Black()} {} 


        RGBEffect(RGBEffect const & from):
            kind{from.kind}, speed{from.speed}, duration{from.duration}, color{from.color} {
            if (kind == Kind::Rainbow)
                rainbow = from.rainbow;
        }

        static RGBEffect Off() { return RGBEffect{}; }

        static RGBEffect Solid(platform::Color color, uint8_t speed = 1, uint16_t duration = 0) {
            RGBEffect result(Kind::Solid, speed, duration);
            result.color = color;
            return result;
        }

        static RGBEffect Solid(uint8_t r, uint8_t g, uint8_t b, uint8_t speed = 1, uint16_t duration = 0) {
            RGBEffect result(Kind::Solid, speed, duration);
            result.color.r = r;
            result.color.g = g;
            result.color.b = b;
            return result;
        }

        static RGBEffect Breathe(platform::Color color, uint8_t speed = 1, uint16_t duration = 0) {
            RGBEffect result(Kind::Breathe, speed, duration);
            result.color = color;
            return result;
        }

        static RGBEffect Rainbow(uint8_t hue, uint8_t step, uint8_t speed = 1, uint8_t brightness = 255, uint16_t duration = 0) {
            RGBEffect result(Kind::Rainbow, speed, duration);
            result.rainbow.hue = hue;
            result.rainbow.step = step;
            result.rainbow.brightness = brightness;
            return result;
        }

        bool active() const { return kind != Kind::Off; }

        void turnOff() {
            kind = Kind::Off;
        }

        platform::Color nextColor(platform::Color const & last) {
            switch (kind) {
                case Kind::Solid:
                    // always return the 
                    return color;
                case Kind::Breathe: 
                    if (last == color)
                        return platform::Color::Black();
                    else
                        return color;
                case Kind::Rainbow:
                    rainbow.hue += rainbow.step;
                    return platform::Color::HSV(rainbow.hue * 256, 255, rainbow.brightness);
                case Kind::Off:
                default:
                    return platform::Color::Black();
            }
        }

        RGBEffect & operator = (RGBEffect const & other) {
            kind = other.kind;
            speed = other.speed;
            duration = other.duration;
            if (kind == Kind::Rainbow)
                rainbow = other.rainbow;
            else
                color = other.color;
            return *this;
        }

    private:

        RGBEffect(Kind kind, uint8_t speed, uint8_t duration):
            kind{kind}, speed{speed}, duration{duration}, color{platform::Color::Black()} {}
    }); 

    /** Rumbler effect description. 

        All times are specified in frames (60 frames per second). This gives resolution of down to 16ms and up to 4 seconds. Also contains predefined rumble effects, such as Ok (single), Fail (tree rumbles), Attention (log rumble) and nudge (single short pulse). 


     */
    PACKED(class RumblerEffect {
    public:
        uint8_t strength = 0;
        uint8_t timeOn = 0;
        uint8_t timeOff = 0;
        uint8_t cycles = 0;

        RumblerEffect() = default;

        RumblerEffect(uint8_t strength, uint8_t timeOn, uint8_t timeOff, uint8_t cycles):
            strength{strength}, timeOn{timeOn}, timeOff{timeOff}, cycles{cycles} {}

        bool active() const { return strength != 0; }

        static RumblerEffect Ok() { return RumblerEffect{
            RUMBLER_DEFAULT_STRENGTH, 
            RUMBLER_OK_DURATION,
            0, 
            1}; } 

        static RumblerEffect Fail() { return RumblerEffect{
            RUMBLER_DEFAULT_STRENGTH, 
            RUMBLER_FAIL_DURATION,
            RUMBLER_FAIL_DURATION,
            3}; }

        static RumblerEffect Attention() { return RumblerEffect{
            RUMBLER_DEFAULT_STRENGTH, 
            RUMBLER_ATTENTION_DURATION,
            0, 
            1}; }

        static RumblerEffect Nudge() { return RumblerEffect{
            RUMBLER_DEFAULT_STRENGTH, 
            RUMBLER_NUDGE_DURATION,
            0, 
            1}; }

        static RumblerEffect Off() { return RumblerEffect{}; }

    });

} // namespace rckid