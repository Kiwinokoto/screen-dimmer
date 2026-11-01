#include "../DimmerLogic.hpp"

#include <cassert>

int main() {
    using namespace screen_dimmer;

    static_assert(ClampBrightness(-15) == 10);
    static_assert(ClampBrightness(37) == 37);
    static_assert(ClampBrightness(110) == 100);
    static_assert(OverlayAlpha(100) == 0);
    static_assert(OverlayAlpha(10) == 229);

    DimmerState state;
    assert(state.brightness == 55 && !state.enabled);
    state.SetBrightness(37);
    assert(state.brightness == 37 && state.enabled);
    state.Toggle();
    assert(!state.enabled && state.brightness == 37);
    state.Toggle();
    assert(state.enabled && state.brightness == 37);
    return 0;
}
