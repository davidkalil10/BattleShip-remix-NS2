/**
 * remix_hud_layer_rules.c - see remix_hud_layer_rules.h.
 */

#include "remix_hud_layer_rules.h"

#include <string.h>

#include "remix_font_layout.h"

static int slotCountOfKind(int kind)
{
    switch (kind)
    {
    case REMIX_HUD_SLOT_TEXT:
        return REMIX_HUD_TEXT_SLOTS;

    case REMIX_HUD_SLOT_RECT:
        return REMIX_HUD_RECT_SLOTS;

    case REMIX_HUD_SLOT_BAR:
        return REMIX_HUD_BAR_SLOTS;

    default:
        return 0;
    }
}

static int rangeIsFree(const uint8_t *slots, int layer, int first_slot, int count)
{
    int slot;

    for (slot = first_slot; slot < first_slot + count; slot++)
    {
        if ((slots[slot] != REMIX_HUD_LAYER_NONE) && (slots[slot] != layer))
        {
            return 0;
        }
    }
    return 1;
}

static float anchorFraction(int align)
{
    switch (align)
    {
    case REMIX_HUD_ALIGN_RIGHT:
        return 1.0f;

    case REMIX_HUD_ALIGN_CENTER:
        return 0.5f;

    default:
        return 0.0f;
    }
}

void remix_hud_routes_clear(RemixHudRoutes *routes)
{
    memset(routes, 0, sizeof(*routes));
}

int remix_hud_routes_open_layer(RemixHudRoutes *routes)
{
    if (routes->layer_count >= REMIX_HUD_LAYERS_MAX)
    {
        return REMIX_HUD_LAYER_NONE;
    }
    routes->layer_count++;
    return routes->layer_count;
}

int remix_hud_routes_bind(RemixHudRoutes *routes, int layer, int kind, int first_slot, int count)
{
    int slot_count = slotCountOfKind(kind);

    if ((layer <= REMIX_HUD_LAYER_NONE) || (layer > routes->layer_count))
    {
        return 0;
    }
    if ((first_slot < 0) || (count <= 0) || (count > slot_count - first_slot))
    {
        return 0;
    }
    if (!rangeIsFree(routes->layers[kind], layer, first_slot, count))
    {
        return 0;
    }
    memset(routes->layers[kind] + first_slot, layer, (size_t)count);
    return 1;
}

int remix_hud_routes_layer_of(const RemixHudRoutes *routes, int kind, int slot)
{
    if ((slot < 0) || (slot >= slotCountOfKind(kind)))
    {
        return REMIX_HUD_LAYER_NONE;
    }
    return routes->layers[kind][slot];
}

int remix_hud_layer_put_is_live(uint32_t put_tic, uint32_t now_tic, uint32_t frame_interval)
{
    uint32_t life = (frame_interval == 0) ? 1 : frame_interval;

    return ((uint32_t)(now_tic - put_tic) <= life) ? 1 : 0;
}

float remix_hud_layer_bar_fill_width(float fraction, float width)
{
    float held = (fraction < 0.0f) ? 0.0f : ((fraction > 1.0f) ? 1.0f : fraction);

    return (float)(int)(held * width);
}

static int heldTo(int value, int low, int high)
{
    return (value < low) ? low : ((value > high) ? high : value);
}

int remix_hud_layer_rect_pixels(float x, float y, float width, float height, RemixHudLayerPixelRect *pixels)
{
    int ulx = (int)x;
    int uly = (int)y;

    pixels->ulx = heldTo(ulx, 0, REMIX_HUD_LAYER_FRAME_WIDTH);
    pixels->uly = heldTo(uly, 0, REMIX_HUD_LAYER_FRAME_HEIGHT);
    pixels->lrx = heldTo(ulx + (int)width, 0, REMIX_HUD_LAYER_FRAME_WIDTH);
    pixels->lry = heldTo(uly + (int)height, 0, REMIX_HUD_LAYER_FRAME_HEIGHT);

    return ((pixels->lrx > pixels->ulx) && (pixels->lry > pixels->uly)) ? 1 : 0;
}

int remix_hud_layer_place_glyphs(const RemixHudLayerString *string, const RemixHudLayerFont *font, RemixHudLayerGlyph *glyphs,
                                 int capacity)
{
    const char *text = string->text;
    float squeeze = remix_font_layout_fit_factor(text, font->widths, font->max_character, string->scale, string->max_width);
    float width = remix_font_layout_aligned_width(text, font->widths, font->max_character, string->scale) * squeeze;
    float cursor = string->x - (width * anchorFraction(string->align));
    int count = 0;

    for (; (*text != 0) && (count < capacity); text++)
    {
        int character = (unsigned char)*text;

        if (remix_font_layout_has_glyph(character, font->max_character))
        {
            RemixHudLayerGlyph *glyph = &glyphs[count++];

            glyph->character = character;
            glyph->x = cursor;
            glyph->y = string->y + ((float)(REMIX_HUD_LAYER_LINE_HEIGHT - (int)font->heights[character]) * string->scale);
            glyph->scalex = string->scale * squeeze * string->glyph_scale;
            glyph->scaley = string->scale * string->glyph_scale;
        }
        cursor += remix_font_layout_advance(character, font->widths, font->max_character, string->scale) * squeeze;
    }
    return count;
}
