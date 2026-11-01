#pragma once

namespace screen_dimmer {

constexpr int ClampBrightness(int percent) {
    return percent < 10 ? 10 : (percent > 100 ? 100 : percent);
}

// The overlay is black: 100% apparent brightness means fully transparent.
constexpr unsigned char OverlayAlpha(int brightness) {
    return static_cast<unsigned char>((100 - ClampBrightness(brightness)) * 255 / 100);
}

struct DimmerState {
    int brightness = 55;
    bool enabled = false;

    void SetBrightness(int percent) {
        brightness = ClampBrightness(percent);
        enabled = true;
    }

    void Toggle() { enabled = !enabled; }
};

}  // namespace screen_dimmer
