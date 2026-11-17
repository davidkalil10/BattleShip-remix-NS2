/**
 * remix_font.h - metrics of the Smash Remix HUD font.
 *
 * Remix draws every HUD string with the "Kabel" bitmap font (File.CUSTOM_FONT). The glyph sprites are drawn by the text
 * layers from the font file of the .o2r (decomp/src/if/ifremixtextlayer.c); the advance widths and heights come from the
 * Remix RAM image (Render.character_offsets_custom). Nothing of the ROM is stored in the repository.
 */

#ifndef PORT_REMIX_REMIX_FONT_H
#define PORT_REMIX_REMIX_FONT_H

#ifdef REGION_REMIX

#include <cstdint>
#include <string>

namespace RemixFont {

/**
 * Reads the glyph width and height tables from the Remix RAM image on first use.
 *
 * @return true when the tables are available; false when the RAM image has none.
 */
bool EnsureMetrics();

/** Table width of every character code 0..MaxCharacter() (Render.character_offsets_custom byte 0); filled by EnsureMetrics. */
const uint8_t *Widths();

/** Table height of every character code 0..MaxCharacter() (Render.character_offsets_custom byte 1); filled by EnsureMetrics. */
const uint8_t *Heights();

/** Last character code of the font. */
int MaxCharacter();

/**
 * Width of a string as Render.draw_string_ lays it out.
 *
 * @param text Text to measure.
 * @param scale Font scale (1.0 is a 16 pixel glyph).
 * @return Width in N64 screen units (one font pixel is one unit at scale 1.0).
 */
float Measure(const std::string &text, float scale);

} // namespace RemixFont

#endif

#endif
