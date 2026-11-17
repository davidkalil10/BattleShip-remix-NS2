/**
 * remix_hud_layer.h - layers of the remix_hud_text slots: the interface between the slot store (remix_hud_text.cpp) and the
 * game object that draws a layer (decomp/src/if/ifremixtextlayer.c).
 *
 * Smash Remix draws every string as an object of a room (a display link), so the cursor, the tokens and the panels of the
 * scene sort against it. An owner opens a layer for a room and binds its slot ranges to it; the put calls do not change. A
 * bound slot is drawn by the layer object with the game renderer; a slot with no layer is not drawn. Layers and routes last one scene (remix_hud_text_clear forgets them).
 *
 * A bound slot lives in game tics: it is drawn by the frame that follows its put and is gone when the owner stops putting it
 * (remix_hud_layer_put_is_live). remix_hud_text_hide still hides at once.
 */

#ifndef PORT_REMIX_REMIX_HUD_LAYER_H
#define PORT_REMIX_REMIX_HUD_LAYER_H

#include <stdint.h>

#include "remix_hud_layer_rules.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Most rectangles one layer draws: every rectangle slot, plus the fill and the flash of every bar slot. */
#define REMIX_HUD_LAYER_RECTS_MAX (REMIX_HUD_RECT_SLOTS + (2 * REMIX_HUD_BAR_SLOTS))
/** Most glyphs of one label. */
#define REMIX_HUD_LAYER_GLYPHS_MAX (REMIX_HUD_TEXT_LENGTH - 1)

/** One filled rectangle in N64 screen units, colour as 0xRRGGBBAA. */
typedef struct RemixHudLayerRect
{
    float x;
    float y;
    float width;
    float height;
    uint32_t rgba;
} RemixHudLayerRect;

/**
 * Opens a layer for the current scene.
 *
 * @return Its id, or REMIX_HUD_LAYER_NONE when the scene has no layer left or the font tables are missing (both go to the
 *         log).
 */
int remix_hud_layer_open(void);

/**
 * Binds a slot range to a layer (remix_hud_routes_bind). A refused range goes to the log and is not drawn.
 *
 * @param layer Id from remix_hud_layer_open.
 * @param kind RemixHudSlotKind of the slots.
 * @param first_slot First slot of the range.
 * @param count Number of slots.
 * @return 1 when the range is bound, else 0.
 */
int remix_hud_layer_bind(int layer, int kind, int first_slot, int count);

/**
 * Rectangles a layer draws this frame, in draw order: its live rectangle slots by slot, then the fill and the white flash
 * of its live bar slots by slot.
 *
 * @param layer Layer id.
 * @param rects Receives the rectangles.
 * @param capacity Entries of rects (REMIX_HUD_LAYER_RECTS_MAX holds them all).
 * @return Number of rectangles written.
 */
int remix_hud_layer_rects(int layer, RemixHudLayerRect *rects, int capacity);

/**
 * Glyphs of one label slot when the layer draws it this frame.
 *
 * @param layer Layer id.
 * @param slot Label slot.
 * @param rgba Receives the label colour as 0xRRGGBBAA.
 * @param glyphs Receives the glyphs (remix_hud_layer_place_glyphs).
 * @param capacity Entries of glyphs (REMIX_HUD_LAYER_GLYPHS_MAX holds a whole label).
 * @return Number of glyphs written; 0 when the slot belongs to another layer or to none, is hidden or was not put for the
 *         frame being drawn.
 */
int remix_hud_layer_label(int layer, int slot, uint32_t *rgba, RemixHudLayerGlyph *glyphs, int capacity);

#ifdef __cplusplus
}
#endif

#endif
