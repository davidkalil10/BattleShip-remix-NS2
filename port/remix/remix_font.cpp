/**
 * remix_font.cpp - see remix_font.h.
 */

#include "remix_font.h"

#ifdef REGION_REMIX

#include <array>
#include <cstdint>

#include "remix_font_decode.h"
#include "remix_font_layout.h"
#include "remix_ram_image.h"
#include "remix_symbols.h"

namespace {

constexpr int kSpaceCharacter = 0x20;
constexpr uint32_t kWidthEntrySize = 8;
constexpr uint32_t kHeightByteOffset = 1;

enum class FontState { NotLoaded, Ready, Failed };

FontState sMetricsState = FontState::NotLoaded;
std::array<uint8_t, REMIX_FONT_LAST_CHARACTER + 1> sWidths{};
std::array<uint8_t, REMIX_FONT_LAST_CHARACTER + 1> sHeights{};

void ReadWidths() {
    for (int character = kSpaceCharacter; character <= REMIX_FONT_LAST_CHARACTER; character++) {
        const uint32_t entry = REMIX_SYM_Render__character_offsets_custom +
                               kWidthEntrySize * static_cast<uint32_t>(character - kSpaceCharacter);
        sWidths[character] = static_cast<uint8_t>(remix_ram_image_read_byte(entry));
        sHeights[character] = static_cast<uint8_t>(remix_ram_image_read_byte(entry + kHeightByteOffset));
    }
}

} // namespace

namespace RemixFont {

bool EnsureMetrics() {
    if (sMetricsState == FontState::NotLoaded) {
        ReadWidths();
        sMetricsState = (sWidths['A'] != 0) ? FontState::Ready : FontState::Failed;
    }
    return sMetricsState == FontState::Ready;
}

const uint8_t *Widths() {
    return sWidths.data();
}

const uint8_t *Heights() {
    return sHeights.data();
}

int MaxCharacter() {
    return REMIX_FONT_LAST_CHARACTER;
}

float Measure(const std::string &text, float scale) {
    return remix_font_layout_aligned_width(text.c_str(), sWidths.data(), REMIX_FONT_LAST_CHARACTER, scale);
}

} // namespace RemixFont

#endif
