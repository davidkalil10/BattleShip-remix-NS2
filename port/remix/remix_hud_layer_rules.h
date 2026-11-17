/**
 * remix_hud_layer_rules.h - pure rules of the Remix text layers (remix_hud_layer.h): which layer draws a slot, how long a put
 * lives and where every glyph of a string goes. No game, ImGui or libultraship dependency, so the headless unit tests check
 * them.
 *
 * A layer is a game object in the display link (room) of the asm; the slots bound to it are drawn
 * by the game renderer. The glyph placement is Render.draw_string_ and Render.apply_max_width_ (Render.asm 1569-1846).
 */

#ifndef PORT_REMIX_REMIX_HUD_LAYER_RULES_H
#define PORT_REMIX_REMIX_HUD_LAYER_RULES_H

#include <stdint.h>

#include "remix_hud_text.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Layer id of a slot that no layer draws. */
#define REMIX_HUD_LAYER_NONE 0
/** Most layers one scene can open; ids go from 1 to this. */
#define REMIX_HUD_LAYERS_MAX 8
/** Height of the tallest glyph in font units: a shorter glyph is dropped by the difference (max height 0x0E in draw_string_). */
#define REMIX_HUD_LAYER_LINE_HEIGHT 14

typedef enum RemixHudSlotKind
{
    REMIX_HUD_SLOT_TEXT,
    REMIX_HUD_SLOT_RECT,
    REMIX_HUD_SLOT_BAR,
    REMIX_HUD_SLOT_KINDS
} RemixHudSlotKind;

/** Layer of every slot of each slot kind (a row is as long as the largest kind), and how many layers the scene opened. */
typedef struct RemixHudRoutes
{
    uint8_t layers[REMIX_HUD_SLOT_KINDS][REMIX_HUD_TEXT_SLOTS];
    int layer_count;
} RemixHudRoutes;

/** Arguments of one string, the ones Render.draw_string_ and Render.apply_max_width_ take. */
typedef struct RemixHudLayerString
{
    const char *text;
    float x;           /* anchor X in N64 screen units */
    float y;           /* top of the glyph box */
    float scale;
    int align;         /* REMIX_HUD_ALIGN_* */
    float max_width;   /* zero or less: no limit */
    float glyph_scale; /* size multiplier of each glyph sprite, the advances keep the plain scale */
} RemixHudLayerString;

/** Width and height table of the font, indexed by character code 0..max_character (Render.character_offsets bytes 0 and 1). */
typedef struct RemixHudLayerFont
{
    const uint8_t *widths;
    const uint8_t *heights;
    int max_character;
} RemixHudLayerFont;

/** One glyph sprite of a laid out string: its top-left corner and the scale of the 16 x 16 sprite on each axis. */
typedef struct RemixHudLayerGlyph
{
    int character;
    float x;
    float y;
    float scalex;
    float scaley;
} RemixHudLayerGlyph;

/** Forgets every layer and route. Done at each scene change. */
void remix_hud_routes_clear(RemixHudRoutes *routes);

/**
 * Opens the next layer of the scene.
 *
 * @return Its id (1..REMIX_HUD_LAYERS_MAX), or REMIX_HUD_LAYER_NONE when the scene already has REMIX_HUD_LAYERS_MAX layers.
 */
int remix_hud_routes_open_layer(RemixHudRoutes *routes);

/**
 * Binds count slots from first_slot to a layer. A slot has one layer: when the layer was not opened, the range leaves the
 * slots of its kind or one of its slots already belongs to another layer, nothing is bound.
 *
 * @param kind RemixHudSlotKind of the slots.
 * @return 1 when the whole range is bound, else 0.
 */
int remix_hud_routes_bind(RemixHudRoutes *routes, int layer, int kind, int first_slot, int count);

/**
 * Layer that draws a slot.
 *
 * @return The layer id, or REMIX_HUD_LAYER_NONE for an unbound slot, an unknown kind or a slot outside its kind.
 */
int remix_hud_routes_layer_of(const RemixHudRoutes *routes, int kind, int slot);

/**
 * Whether a put is still shown by its layer. The game counts a tic after each update and draws when the count is a multiple
 * of the frame interval, so a put made in an update is at most frame_interval tics old at the next draw; an owner that stops
 * putting is gone one draw later.
 *
 * @param put_tic Tic count read when the slot was put.
 * @param now_tic Tic count read by the display function.
 * @param frame_interval Updates per drawn frame (zero counts as one).
 * @return 1 while the put is shown, else 0.
 */
int remix_hud_layer_put_is_live(uint32_t put_tic, uint32_t now_tic, uint32_t frame_interval);

/**
 * Width of the filled part of a progress bar (KingOfTheHill.asm update_indicators_): the whole units of fraction * width,
 * with the fraction held to 0..1.
 */
float remix_hud_layer_bar_fill_width(float fraction, float width);

#define REMIX_HUD_LAYER_FRAME_WIDTH 320
#define REMIX_HUD_LAYER_FRAME_HEIGHT 240

/** Whole-pixel corners of a rectangle on the 320x240 frame; the lower right corner is exclusive. */
typedef struct RemixHudLayerPixelRect {
    int ulx;
    int uly;
    int lrx;
    int lry;
} RemixHudLayerPixelRect;

/**
 * Pixels a rectangle fills (Render.draw_rectangle_ keeps the corner and the size as integers): the corner and the size
 * are truncated one by one and the result is cut to the 320x240 frame, since the fill command has no signed coordinates.
 *
 * @param pixels Receives the corners.
 * @return 1 when something is left to fill, else 0.
 */
int remix_hud_layer_rect_pixels(float x, float y, float width, float height, RemixHudLayerPixelRect *pixels);

/**
 * Lays a string out glyph by glyph. The start X is the anchor less the aligned width times the squeeze (nothing for LEFT,
 * all of it for RIGHT, half for CENTER); a glyph sits at the running X and at y + (14 - glyph height) * scale (the squeeze
 * does not shorten the drop, see deviations.md); it is
 * scale * squeeze * glyph_scale wide and scale * glyph_scale high per sprite unit; the running X advances by the font advance
 * times the squeeze. Spaces and characters without a glyph only advance.
 *
 * @param string Arguments of the string.
 * @param font Width and height tables.
 * @param glyphs Receives the glyphs in text order.
 * @param capacity Entries of glyphs; a longer string is cut there.
 * @return Number of glyphs written.
 */
int remix_hud_layer_place_glyphs(const RemixHudLayerString *string, const RemixHudLayerFont *font, RemixHudLayerGlyph *glyphs,
                                 int capacity);

#ifdef __cplusplus
}
#endif

#endif
