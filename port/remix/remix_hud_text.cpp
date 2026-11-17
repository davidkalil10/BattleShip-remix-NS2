/**
 * remix_hud_text.cpp - see remix_hud_text.h.
 *
 * The overlay is a GuiWindow without an ImGui window of its own, like remix_input_display: it draws on the foreground
 * draw list. A label stays visible for the ImGui frame it was put in and the next one. A slot bound to a layer
 * (remix_hud_layer.h) is skipped here and handed to the game object of the layer instead.
 */

#include "remix_hud_text.h"

#ifdef REGION_REMIX

#include <imgui.h>
#include <libultraship/libultraship.h>
#include <ship/Context.h>
#include <ship/config/ConsoleVariable.h>
#include "gui/Compat.h"

#include "remix_font.h"
#include "remix_hud_layer.h"
#include "widescreen/widescreen.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <spdlog/spdlog.h>

extern "C" {
/* Game tic count and updates per drawn frame of the running scene (decomp/src/sys/taskman.c). */
extern uint32_t dSYTaskmanUpdateCount;
extern uint16_t sSYTaskmanFrameInterval;
}

namespace {

constexpr float kN64ScreenWidth = 320.0f;
constexpr float kN64ScreenHeight = 240.0f;
constexpr float kDefaultOverscan = 10.0f;
constexpr int kLabelLifeFrames = 2;
constexpr uint32_t kShadowColor = IM_COL32(0, 0, 0, 200);

struct HudLabel {
    bool active = false;
    int lastFrame = 0;
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
    int lastFrame = 0;
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
    int lastFrame = 0;
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
int sCurrentFrame = 0;

/** Whether a layer draws the slot, so the overlay leaves it alone. */
bool IsOnLayer(int kind, int slot) {
    return remix_hud_routes_layer_of(&sRoutes, kind, slot) != REMIX_HUD_LAYER_NONE;
}

/** Whether the layer draws the slot in the frame being drawn: it is bound to that layer, not hidden, and its put is fresh. */
bool IsDrawnByLayer(int layer, int kind, int slot, bool active, uint32_t putTic) {
    return active && (remix_hud_routes_layer_of(&sRoutes, kind, slot) == layer) &&
           (remix_hud_layer_put_is_live(putTic, dSYTaskmanUpdateCount, sSYTaskmanFrameInterval) != 0);
}

/** SSB64_REMIX_TEXT_OVERLAY=1 keeps every slot on the overlay, to compare it with the layers in the same binary. */
bool IsOverlayForced() {
    static const bool forced = [] {
        const char *value = std::getenv("SSB64_REMIX_TEXT_OVERLAY");
        return (value != nullptr) && (value[0] == '1');
    }();

    return forced;
}

/** Overscan margin in N64 units that Gui::DrawGame crops from every side of the presented frame (zero when disabled). */
ImVec2 OverscanMargin() {
    const auto cvars = Ship::Context::GetInstance()->GetConsoleVariables();
    const bool pixelPerfect = cvars->GetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".Enabled", 0) &&
                              cvars->GetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".PixelPerfectMode", 0);

    if (pixelPerfect || !cvars->GetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".Overscan.Enabled", 1)) {
        return ImVec2(0.0f, 0.0f);
    }
    return ImVec2(cvars->GetFloat(CVAR_PREFIX_ADVANCED_RESOLUTION ".Overscan.Horizontal", kDefaultOverscan),
                  cvars->GetFloat(CVAR_PREFIX_ADVANCED_RESOLUTION ".Overscan.Vertical", kDefaultOverscan));
}

/**
 * True when Gui::DrawGame presents the game over the whole work area: neither the 4:3 aspect correction nor the stretched-4:3
 * Low Resolution Mode is on. Both are latched at boot (port.cpp), so toggling widescreen changes this only after a restart.
 */
bool GameImageFillsWorkArea() {
    const auto cvars = Ship::Context::GetInstance()->GetConsoleVariables();

    return !cvars->GetInteger(CVAR_PREFIX_ADVANCED_RESOLUTION ".Enabled", 0) && cvars->GetInteger("gLowResMode", 0) != 1;
}

/** Size of the window area the 2D frame covers: the whole work area when the image fills it with 2D stretched, else the largest centred 4:3. */
ImVec2 FrameSize(const ImGuiViewport *viewport) {
    if (GameImageFillsWorkArea() && (port_widescreen_stretches_2d() != 0)) {
        return viewport->WorkSize;
    }
    const float fitScale = std::min(viewport->WorkSize.x / kN64ScreenWidth, viewport->WorkSize.y / kN64ScreenHeight);
    return ImVec2(kN64ScreenWidth * fitScale, kN64ScreenHeight * fitScale);
}

/**
 * Maps N64 screen units to window pixels: the area the 2D frame covers (FrameSize) shows the frame without its overscan margin,
 * exactly as the presented game image does. The two axes scale slightly differently (the margin is not 4:3).
 */
void GameRect(const ImGuiViewport *viewport, ImVec2 *origin, ImVec2 *scale) {
    const ImVec2 frameSize = FrameSize(viewport);
    const float rectWidth = frameSize.x;
    const float rectHeight = frameSize.y;
    const ImVec2 margin = OverscanMargin();

    scale->x = rectWidth / (kN64ScreenWidth - 2.0f * margin.x);
    scale->y = rectHeight / (kN64ScreenHeight - 2.0f * margin.y);
    origin->x = viewport->WorkPos.x + (viewport->WorkSize.x - rectWidth) * 0.5f - margin.x * scale->x;
    origin->y = viewport->WorkPos.y + (viewport->WorkSize.y - rectHeight) * 0.5f - margin.y * scale->y;
}

ImU32 ColorOf(uint32_t rgba) {
    return IM_COL32((rgba >> 24) & 0xFF, (rgba >> 16) & 0xFF, (rgba >> 8) & 0xFF, rgba & 0xFF);
}

/** Fraction of the string width that lies left of the anchor X for each Render alignment. */
float AnchorFraction(int align) {
    switch (align) {
        case REMIX_HUD_ALIGN_RIGHT:
            return 1.0f;
        case REMIX_HUD_ALIGN_CENTER:
            return 0.5f;
        default:
            return 0.0f;
    }
}

/** Horizontal squeeze that makes the string fit the label max width; 1 when it already fits or has no limit. */
float WidthFactorOf(const HudLabel &label) {
    return RemixFont::FitFactor(label.text, label.scale, label.maxWidth);
}

void DrawLabelWithRemixFont(ImDrawList *drawList, ImVec2 origin, ImVec2 scale, const HudLabel &label) {
    const float widthFactor = WidthFactorOf(label);
    const float textWidth = RemixFont::Measure(label.text, label.scale) * widthFactor * scale.x;
    const ImVec2 position(origin.x + label.x * scale.x - textWidth * AnchorFraction(label.align), origin.y + label.y * scale.y);

    RemixFont::Draw(drawList, position, scale, label.scale, ColorOf(label.rgba), label.text, widthFactor, label.glyphScale);
}

void DrawLabel(ImDrawList *drawList, ImVec2 origin, ImVec2 scale, const HudLabel &label) {
    if (RemixFont::Ensure()) {
        DrawLabelWithRemixFont(drawList, origin, scale, label);
        return;
    }
    const float fontSize = RemixFont::kGlyphHeight * label.scale * scale.y;
    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label.text);
    const ImVec2 position(origin.x + label.x * scale.x - textSize.x * AnchorFraction(label.align), origin.y + label.y * scale.y);
    const float shadow = std::max(1.0f, fontSize * 0.08f);

    drawList->AddText(ImGui::GetFont(), fontSize, ImVec2(position.x + shadow, position.y + shadow), kShadowColor,
                      label.text);
    drawList->AddText(ImGui::GetFont(), fontSize, position, ColorOf(label.rgba), label.text);
}

/** KingOfTheHill.asm update_indicators_: the bar is a plain rectangle trunc(fraction * width) wide, and the white flash covers the whole bar. */
void DrawBar(ImDrawList *drawList, ImVec2 origin, ImVec2 scale, const HudBar &bar) {
    const ImVec2 min(origin.x + bar.x * scale.x, origin.y + bar.y * scale.y);
    const ImVec2 max(min.x + bar.width * scale.x, min.y + bar.height * scale.y);
    const float fillWidth = remix_hud_layer_bar_fill_width(bar.fraction, bar.width);

    if (fillWidth > 0.0f) {
        drawList->AddRectFilled(min, ImVec2(min.x + fillWidth * scale.x, max.y), ColorOf(bar.rgba));
    }
    if (bar.flash > 0.0f) {
        drawList->AddRectFilled(min, max, IM_COL32(255, 255, 255, static_cast<int>(255.0f * std::clamp(bar.flash, 0.0f, 1.0f))));
    }
}

void DrawHudBars(ImDrawList *drawList, ImVec2 origin, ImVec2 scale) {
    for (int slot = 0; slot < REMIX_HUD_BAR_SLOTS; slot++) {
        HudBar &bar = sBars[slot];

        if (!bar.active || IsOnLayer(REMIX_HUD_SLOT_BAR, slot)) {
            continue;
        }
        if (sCurrentFrame - bar.lastFrame > kLabelLifeFrames) {
            bar.active = false;
            continue;
        }
        DrawBar(drawList, origin, scale, bar);
    }
}

void DrawHudRects(ImDrawList *drawList, ImVec2 origin, ImVec2 scale) {
    for (int slot = 0; slot < REMIX_HUD_RECT_SLOTS; slot++) {
        HudRect &rect = sRects[slot];

        if (!rect.active || IsOnLayer(REMIX_HUD_SLOT_RECT, slot)) {
            continue;
        }
        if (sCurrentFrame - rect.lastFrame > kLabelLifeFrames) {
            rect.active = false;
            continue;
        }
        const ImVec2 min(origin.x + rect.x * scale.x, origin.y + rect.y * scale.y);
        drawList->AddRectFilled(min, ImVec2(min.x + rect.width * scale.x, min.y + rect.height * scale.y), ColorOf(rect.rgba));
    }
}

/** Whether the port menu is open: the HUD overlay stays under it instead of drawing over its widgets. */
bool IsPortMenuOpen() {
    auto context = Ship::Context::GetInstance();
    auto menu = (context && context->GetWindow() && context->GetWindow()->GetGui()) ? context->GetWindow()->GetGui()->GetMenu() : nullptr;

    return menu && menu->IsVisible();
}

void DrawHudLabels() {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImDrawList *drawList = ImGui::GetForegroundDrawList();
    ImVec2 origin;
    ImVec2 scale;

    sCurrentFrame = ImGui::GetFrameCount();
    if (IsPortMenuOpen()) {
        return;
    }
    GameRect(viewport, &origin, &scale);
    DrawHudRects(drawList, origin, scale);
    DrawHudBars(drawList, origin, scale);
    for (int slot = 0; slot < REMIX_HUD_TEXT_SLOTS; slot++) {
        HudLabel &label = sLabels[slot];

        if (!label.active || IsOnLayer(REMIX_HUD_SLOT_TEXT, slot)) {
            continue;
        }
        if (sCurrentFrame - label.lastFrame > kLabelLifeFrames) {
            label.active = false;
            continue;
        }
        DrawLabel(drawList, origin, scale, label);
    }
}

class RemixHudTextWindow : public Ship::GuiWindow {
  public:
    RemixHudTextWindow() : Ship::GuiWindow("", true, "Remix HUD Text") {
    }

    /** Overrides GuiWindow::Draw, which opens an ImGui window. The overlay has no window of its own. */
    void Draw() override {
        DrawElement();
    }

    void DrawElement() override {
        DrawHudLabels();
    }

    void InitElement() override {
    }

    void UpdateElement() override {
    }
};

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
    label.lastFrame = sCurrentFrame;
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
    bar.lastFrame = sCurrentFrame;
    bar.putTic = dSYTaskmanUpdateCount;
}

extern "C" float remix_hud_text_measure(const char *text, float scale) {
    if (text == nullptr || !RemixFont::Ensure()) {
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
    rect.lastFrame = sCurrentFrame;
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
    if (IsOverlayForced()) {
        return REMIX_HUD_LAYER_NONE;
    }
    if (!RemixFont::EnsureMetrics()) {
        spdlog::warn("remix_hud_layer: Remix RAM image has no glyph widths; the text stays on the overlay");
        return REMIX_HUD_LAYER_NONE;
    }
    const int layer = remix_hud_routes_open_layer(&sRoutes);

    if (layer == REMIX_HUD_LAYER_NONE) {
        spdlog::error("remix_hud_layer: the scene already has {} layers; the text stays on the overlay", REMIX_HUD_LAYERS_MAX);
    }
    return layer;
}

extern "C" int remix_hud_layer_bind(int layer, int kind, int first_slot, int count) {
    if (remix_hud_routes_bind(&sRoutes, layer, kind, first_slot, count) == 0) {
        spdlog::error("remix_hud_layer: layer {} cannot take slots {}..{} of kind {} (outside the kind, or owned by another "
                      "layer); they stay on the overlay",
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

std::shared_ptr<Ship::GuiWindow> remix_hud_text_make_window() {
    return std::make_shared<RemixHudTextWindow>();
}

#endif
