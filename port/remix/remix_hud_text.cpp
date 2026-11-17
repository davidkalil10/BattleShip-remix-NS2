/**
 * remix_hud_text.cpp - see remix_hud_text.h.
 *
 * The store keeps the last put of every label, rectangle and bar slot. Nothing is drawn here: the game object of the layer
 * a slot is bound to (remix_hud_layer.h) reads the live puts of its slots every frame.
 */

#include "remix_hud_text.h"

#ifdef REGION_REMIX

#include "remix_font.h"
#include "remix_hud_layer.h"

#include <array>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <spdlog/spdlog.h>

extern "C" {
/* Game tic count and updates per drawn frame of the running scene (decomp/src/sys/taskman.c). */
extern uint32_t dSYTaskmanUpdateCount;
extern uint16_t sSYTaskmanFrameInterval;
}

namespace {

struct HudLabel {
    bool active = false;
    uint32_t putTic = 0;
    char text[REMIX_HUD_TEXT_LENGTH] = {};
    float x = 0.0f;
    float y = 0.0f;
    uint32_t rgba = 0xFFFFFFFF;
    float scale = 1.0f;
    int align = REMIX_HUD_ALIGN_LEFT;
    float maxWidth = 0.0f;
    float glyphScale = 1.0f;
};

struct HudBar {
    bool active = false;
    uint32_t putTic = 0;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float fraction = 0.0f;
    uint32_t rgba = 0xFFFFFFFF;
    float flash = 0.0f;
};

struct HudRect {
    bool active = false;
    uint32_t putTic = 0;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    uint32_t rgba = 0xFFFFFFFF;
};

std::array<HudRect, REMIX_HUD_RECT_SLOTS> sRects;
std::array<HudLabel, REMIX_HUD_TEXT_SLOTS> sLabels;
std::array<HudBar, REMIX_HUD_BAR_SLOTS> sBars;
RemixHudRoutes sRoutes = {};

/** Whether the layer draws the slot in the frame being drawn: it is bound to that layer, not hidden, and its put is fresh. */
bool IsDrawnByLayer(int layer, int kind, int slot, bool active, uint32_t putTic) {
    return active && (remix_hud_routes_layer_of(&sRoutes, kind, slot) == layer) &&
           (remix_hud_layer_put_is_live(putTic, dSYTaskmanUpdateCount, sSYTaskmanFrameInterval) != 0);
}

} // namespace

extern "C" void remix_hud_text_put(int slot, const char *text, float x, float y, uint32_t rgba, float scale, int align) {
    if (slot < 0 || slot >= REMIX_HUD_TEXT_SLOTS || text == nullptr) {
        return;
    }
    HudLabel &label = sLabels[slot];
    std::strncpy(label.text, text, REMIX_HUD_TEXT_LENGTH - 1);
    label.text[REMIX_HUD_TEXT_LENGTH - 1] = '\0';
    label.x = x;
    label.y = y;
    label.rgba = rgba;
    label.scale = scale;
    label.align = align;
    label.maxWidth = 0.0f;
    label.glyphScale = 1.0f;
    label.active = true;
    label.putTic = dSYTaskmanUpdateCount;
}

extern "C" void remix_hud_text_put_fit(int slot, const char *text, float x, float y, uint32_t rgba, float scale, int align,
                                       float max_width) {
    remix_hud_text_put(slot, text, x, y, rgba, scale, align);
    if (slot >= 0 && slot < REMIX_HUD_TEXT_SLOTS && text != nullptr) {
        sLabels[slot].maxWidth = max_width;
    }
}

extern "C" void remix_hud_text_put_fit_glyph_scale(int slot, const char *text, float x, float y, uint32_t rgba, float scale,
                                                   int align, float max_width, float glyph_scale) {
    remix_hud_text_put_fit(slot, text, x, y, rgba, scale, align, max_width);
    if (slot >= 0 && slot < REMIX_HUD_TEXT_SLOTS && text != nullptr) {
        sLabels[slot].glyphScale = glyph_scale;
    }
}

extern "C" void remix_hud_text_putf(int slot, float x, float y, uint32_t rgba, float scale, int align, const char *format,
                                    ...) {
    char text[REMIX_HUD_TEXT_LENGTH];
    va_list arguments;

    va_start(arguments, format);
    std::vsnprintf(text, sizeof(text), format, arguments);
    va_end(arguments);
    remix_hud_text_put(slot, text, x, y, rgba, scale, align);
}

extern "C" void remix_hud_bar_put(int slot, float x, float y, float width, float height, float fraction, uint32_t rgba,
                                  float flash) {
    if (slot < 0 || slot >= REMIX_HUD_BAR_SLOTS) {
        return;
    }
    HudBar &bar = sBars[slot];
    bar.x = x;
    bar.y = y;
    bar.width = width;
    bar.height = height;
    bar.fraction = fraction;
    bar.rgba = rgba;
    bar.flash = flash;
    bar.active = true;
    bar.putTic = dSYTaskmanUpdateCount;
}

extern "C" float remix_hud_text_measure(const char *text, float scale) {
    if (text == nullptr || !RemixFont::EnsureMetrics()) {
        return 0.0f;
    }
    return RemixFont::Measure(text, scale);
}

extern "C" void remix_hud_rect_put(int slot, float x, float y, float width, float height, uint32_t rgba) {
    if (slot < 0 || slot >= REMIX_HUD_RECT_SLOTS) {
        return;
    }
    HudRect &rect = sRects[slot];
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    rect.rgba = rgba;
    rect.active = true;
    rect.putTic = dSYTaskmanUpdateCount;
}

extern "C" void remix_hud_text_hide(int slot) {
    if (slot < 0 || slot >= REMIX_HUD_TEXT_SLOTS) {
        return;
    }
    sLabels[slot].active = false;
}

extern "C" void remix_hud_text_clear(void) {
    for (HudLabel &label : sLabels) {
        label.active = false;
    }
    for (HudBar &bar : sBars) {
        bar.active = false;
    }
    for (HudRect &rect : sRects) {
        rect.active = false;
    }
    remix_hud_routes_clear(&sRoutes);
}

extern "C" int remix_hud_layer_open(void) {
    if (!RemixFont::EnsureMetrics()) {
        spdlog::warn("remix_hud_layer: Remix RAM image has no glyph widths; the text is not drawn");
        return REMIX_HUD_LAYER_NONE;
    }
    const int layer = remix_hud_routes_open_layer(&sRoutes);

    if (layer == REMIX_HUD_LAYER_NONE) {
        spdlog::error("remix_hud_layer: the scene already has {} layers; the text is not drawn", REMIX_HUD_LAYERS_MAX);
    }
    return layer;
}

extern "C" int remix_hud_layer_bind(int layer, int kind, int first_slot, int count) {
    if (remix_hud_routes_bind(&sRoutes, layer, kind, first_slot, count) == 0) {
        spdlog::error("remix_hud_layer: layer {} cannot take slots {}..{} of kind {} (outside the kind, or owned by another "
                      "layer); they are not drawn",
                      layer, first_slot, first_slot + count - 1, kind);
        return 0;
    }
    return 1;
}

extern "C" int remix_hud_layer_rects(int layer, RemixHudLayerRect *rects, int capacity) {
    int count = 0;

    for (int slot = 0; (slot < REMIX_HUD_RECT_SLOTS) && (count < capacity); slot++) {
        const HudRect &rect = sRects[slot];

        if (IsDrawnByLayer(layer, REMIX_HUD_SLOT_RECT, slot, rect.active, rect.putTic)) {
            rects[count++] = { rect.x, rect.y, rect.width, rect.height, rect.rgba };
        }
    }
    for (int slot = 0; (slot < REMIX_HUD_BAR_SLOTS) && (count + 1 < capacity); slot++) {
        const HudBar &bar = sBars[slot];

        if (!IsDrawnByLayer(layer, REMIX_HUD_SLOT_BAR, slot, bar.active, bar.putTic)) {
            continue;
        }
        const float fillWidth = remix_hud_layer_bar_fill_width(bar.fraction, bar.width);
        const uint32_t flashAlpha = static_cast<uint32_t>(255.0f * std::clamp(bar.flash, 0.0f, 1.0f));

        if (fillWidth > 0.0f) {
            rects[count++] = { bar.x, bar.y, fillWidth, bar.height, bar.rgba };
        }
        if (flashAlpha > 0) {
            rects[count++] = { bar.x, bar.y, bar.width, bar.height, 0xFFFFFF00u | flashAlpha };
        }
    }
    return count;
}

extern "C" int remix_hud_layer_label(int layer, int slot, uint32_t *rgba, RemixHudLayerGlyph *glyphs, int capacity) {
    if (slot < 0 || slot >= REMIX_HUD_TEXT_SLOTS) {
        return 0;
    }
    const HudLabel &label = sLabels[slot];

    if (!IsDrawnByLayer(layer, REMIX_HUD_SLOT_TEXT, slot, label.active, label.putTic)) {
        return 0;
    }
    const RemixHudLayerString string = { label.text, label.x, label.y, label.scale, label.align, label.maxWidth,
                                         label.glyphScale };
    const RemixHudLayerFont font = { RemixFont::Widths(), RemixFont::Heights(), RemixFont::MaxCharacter() };

    *rgba = label.rgba;
    return remix_hud_layer_place_glyphs(&string, &font, glyphs, capacity);
}

#endif
