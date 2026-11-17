/**
 * test_hud_layer_rules.c - unit tests of the Remix text layer rules (port/remix/remix_hud_layer_rules.c): the route table,
 * the life of a put in game tics, the bar fill and the glyph placement of Render.draw_string_ / apply_max_width_.
 */

#include "test_check.h"

#include <string.h>

#include "remix_hud_layer_rules.h"

#define TEST_MAX_CHARACTER 0x80
#define TEST_GLYPH_WIDTH 7
#define TEST_GLYPH_HEIGHT 14
#define TEST_GLYPHS 16

static uint8_t sTestWidths[TEST_MAX_CHARACTER + 1];
static uint8_t sTestHeights[TEST_MAX_CHARACTER + 1];
static const RemixHudLayerFont sTestFont = { sTestWidths, sTestHeights, TEST_MAX_CHARACTER };

/* Every glyph 7 wide and 14 high, except a narrow '!' and a short 'a' (10 high, so dropped by 4). */
static void fillFont(void)
{
    int character;

    memset(sTestWidths, 0, sizeof(sTestWidths));
    memset(sTestHeights, 0, sizeof(sTestHeights));
    for (character = 0x21; character <= TEST_MAX_CHARACTER; character++)
    {
        sTestWidths[character] = TEST_GLYPH_WIDTH;
        sTestHeights[character] = TEST_GLYPH_HEIGHT;
    }
    sTestWidths['!'] = 4;
    sTestHeights['a'] = 10;
}

static RemixHudLayerString makeString(const char *text, float x, float scale, int align)
{
    RemixHudLayerString string = { text, x, 100.0f, scale, align, 0.0f, 1.0f };

    return string;
}

/* Sixteenths of a unit, so the checks compare exact binary fractions as integers. */
static int sixteenths(float value)
{
    return (int)(value * 16.0f);
}

static void testUnboundSlotHasNoLayer(void)
{
    RemixHudRoutes routes;

    remix_hud_routes_clear(&routes);
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 32));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_RECT, 0));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_BAR, 15));
}

static void testLayersAreNumberedFromOneUpToTheLimit(void)
{
    RemixHudRoutes routes;
    int layer;

    remix_hud_routes_clear(&routes);
    for (layer = 1; layer <= REMIX_HUD_LAYERS_MAX; layer++)
    {
        CHECK_EQ(layer, remix_hud_routes_open_layer(&routes));
    }
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_open_layer(&routes));
}

static void testBoundRangeBelongsToItsLayerOnly(void)
{
    RemixHudRoutes routes;
    int first;
    int second;

    remix_hud_routes_clear(&routes);
    first = remix_hud_routes_open_layer(&routes);
    second = remix_hud_routes_open_layer(&routes);
    CHECK_EQ(1, remix_hud_routes_bind(&routes, first, REMIX_HUD_SLOT_TEXT, 32, 3));
    CHECK_EQ(1, remix_hud_routes_bind(&routes, second, REMIX_HUD_SLOT_TEXT, 44, 3));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 31));
    CHECK_EQ(first, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 32));
    CHECK_EQ(first, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 34));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 35));
    CHECK_EQ(second, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 46));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_RECT, 32));
}

static void testEachSlotKindHasItsOwnRoutes(void)
{
    RemixHudRoutes routes;
    int layer;

    remix_hud_routes_clear(&routes);
    layer = remix_hud_routes_open_layer(&routes);
    CHECK_EQ(1, remix_hud_routes_bind(&routes, layer, REMIX_HUD_SLOT_RECT, 40, 6));
    CHECK_EQ(1, remix_hud_routes_bind(&routes, layer, REMIX_HUD_SLOT_BAR, 0, 4));
    CHECK_EQ(layer, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_RECT, 45));
    CHECK_EQ(layer, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_BAR, 3));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 40));
}

static void testRefusedBindLeavesEverySlotAsItWas(void)
{
    RemixHudRoutes routes;
    int first;
    int second;

    remix_hud_routes_clear(&routes);
    first = remix_hud_routes_open_layer(&routes);
    second = remix_hud_routes_open_layer(&routes);
    CHECK_EQ(1, remix_hud_routes_bind(&routes, first, REMIX_HUD_SLOT_TEXT, 34, 2));

    /* overlaps the slots of another layer */
    CHECK_EQ(0, remix_hud_routes_bind(&routes, second, REMIX_HUD_SLOT_TEXT, 32, 3));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 32));
    CHECK_EQ(first, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 34));

    /* outside the slots of the kind, an empty range, a layer that was not opened, an unknown kind */
    CHECK_EQ(0, remix_hud_routes_bind(&routes, first, REMIX_HUD_SLOT_TEXT, REMIX_HUD_TEXT_SLOTS - 1, 2));
    CHECK_EQ(0, remix_hud_routes_bind(&routes, first, REMIX_HUD_SLOT_RECT, REMIX_HUD_RECT_SLOTS, 1));
    CHECK_EQ(0, remix_hud_routes_bind(&routes, first, REMIX_HUD_SLOT_BAR, -1, 2));
    CHECK_EQ(0, remix_hud_routes_bind(&routes, first, REMIX_HUD_SLOT_TEXT, 60, 0));
    CHECK_EQ(0, remix_hud_routes_bind(&routes, second + 1, REMIX_HUD_SLOT_TEXT, 60, 1));
    CHECK_EQ(0, remix_hud_routes_bind(&routes, REMIX_HUD_LAYER_NONE, REMIX_HUD_SLOT_TEXT, 60, 1));
    CHECK_EQ(0, remix_hud_routes_bind(&routes, first, REMIX_HUD_SLOT_KINDS, 0, 1));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 60));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, REMIX_HUD_TEXT_SLOTS - 1));
}

static void testSameLayerMayBindASlotTwiceAndTheLastSlotOfAKind(void)
{
    RemixHudRoutes routes;
    int layer;

    remix_hud_routes_clear(&routes);
    layer = remix_hud_routes_open_layer(&routes);
    CHECK_EQ(1, remix_hud_routes_bind(&routes, layer, REMIX_HUD_SLOT_TEXT, 32, 3));
    CHECK_EQ(1, remix_hud_routes_bind(&routes, layer, REMIX_HUD_SLOT_TEXT, 34, 3));
    CHECK_EQ(1, remix_hud_routes_bind(&routes, layer, REMIX_HUD_SLOT_TEXT, REMIX_HUD_TEXT_SLOTS - 1, 1));
    CHECK_EQ(layer, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 36));
    CHECK_EQ(layer, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, REMIX_HUD_TEXT_SLOTS - 1));
}

static void testLookupOutsideTheSlotsIsNoLayer(void)
{
    RemixHudRoutes routes;
    int layer;

    remix_hud_routes_clear(&routes);
    layer = remix_hud_routes_open_layer(&routes);
    CHECK_EQ(1, remix_hud_routes_bind(&routes, layer, REMIX_HUD_SLOT_BAR, 0, REMIX_HUD_BAR_SLOTS));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_BAR, -1));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_BAR, REMIX_HUD_BAR_SLOTS));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_KINDS, 0));
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, -1, 0));
}

static void testClearForgetsLayersAndRoutes(void)
{
    RemixHudRoutes routes;
    int layer;

    remix_hud_routes_clear(&routes);
    layer = remix_hud_routes_open_layer(&routes);
    remix_hud_routes_bind(&routes, layer, REMIX_HUD_SLOT_TEXT, 32, 8);
    remix_hud_routes_clear(&routes);
    CHECK_EQ(REMIX_HUD_LAYER_NONE, remix_hud_routes_layer_of(&routes, REMIX_HUD_SLOT_TEXT, 32));
    CHECK_EQ(1, remix_hud_routes_open_layer(&routes));
}

static void testPutLivesForTheFrameThatFollowsIt(void)
{
    /* one update per frame: put during the update of tic 10, drawn at count 11, gone when the next update does not put */
    CHECK_EQ(1, remix_hud_layer_put_is_live(10, 11, 1));
    CHECK_EQ(0, remix_hud_layer_put_is_live(10, 12, 1));
    /* a put made while drawing (after the count went up) shows in that frame and the next */
    CHECK_EQ(1, remix_hud_layer_put_is_live(11, 11, 1));
    CHECK_EQ(1, remix_hud_layer_put_is_live(11, 12, 1));
}

static void testPutLivesOneFrameIntervalOfUpdates(void)
{
    /* two updates per frame: a put in either update of the pair reaches the draw */
    CHECK_EQ(1, remix_hud_layer_put_is_live(10, 12, 2));
    CHECK_EQ(1, remix_hud_layer_put_is_live(11, 12, 2));
    CHECK_EQ(0, remix_hud_layer_put_is_live(9, 12, 2));
    CHECK_EQ(1, remix_hud_layer_put_is_live(10, 11, 0));
    CHECK_EQ(0, remix_hud_layer_put_is_live(10, 12, 0));
}

static void testPutOfAnEarlierSceneOrAcrossTheCounterWrapIsJudgedByDistance(void)
{
    /* the count restarts at each scene: a stamp above the count is far away, not live */
    CHECK_EQ(0, remix_hud_layer_put_is_live(5000, 3, 1));
    CHECK_EQ(1, remix_hud_layer_put_is_live(0xFFFFFFFFu, 0, 1));
}

static void testBarFillIsTheWholeUnitsOfTheHeldFraction(void)
{
    CHECK_EQ(20, (int)remix_hud_layer_bar_fill_width(0.5f, 40.0f));
    CHECK_EQ(13, (int)remix_hud_layer_bar_fill_width(0.34f, 40.0f));
    CHECK_EQ(0, (int)remix_hud_layer_bar_fill_width(-1.0f, 40.0f));
    CHECK_EQ(40, (int)remix_hud_layer_bar_fill_width(2.0f, 40.0f));
    CHECK_EQ(0, (int)remix_hud_layer_bar_fill_width(0.01f, 40.0f));
}

static void testLeftAlignedGlyphsStartAtTheAnchorAndAdvance(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString string = makeString("A!B", 50.0f, 1.0f, REMIX_HUD_ALIGN_LEFT);

    fillFont();
    CHECK_EQ(3, remix_hud_layer_place_glyphs(&string, &sTestFont, glyphs, TEST_GLYPHS));
    CHECK_EQ('A', glyphs[0].character);
    CHECK_EQ(50, (int)glyphs[0].x);
    CHECK_EQ('!', glyphs[1].character);
    CHECK_EQ(58, (int)glyphs[1].x);
    CHECK_EQ(63, (int)glyphs[2].x);
    CHECK_EQ(100, (int)glyphs[0].y);
    CHECK_EQ(16, sixteenths(glyphs[0].scalex));
    CHECK_EQ(16, sixteenths(glyphs[0].scaley));
}

static void testSpacesAdvanceByFourAndMakeNoGlyph(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString string = makeString(" A  B", 10.0f, 0.5f, REMIX_HUD_ALIGN_LEFT);

    fillFont();
    CHECK_EQ(2, remix_hud_layer_place_glyphs(&string, &sTestFont, glyphs, TEST_GLYPHS));
    CHECK_EQ(12, (int)glyphs[0].x);
    CHECK_EQ(20, (int)glyphs[1].x);
}

static void testRightAndCentreAnchorsUseTheAlignedWidth(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString right = makeString("AB", 100.0f, 1.0f, REMIX_HUD_ALIGN_RIGHT);
    RemixHudLayerString centre = makeString("AB", 100.0f, 1.0f, REMIX_HUD_ALIGN_CENTER);
    RemixHudLayerString scaled = makeString("AB", 160.0f, 0.875f, REMIX_HUD_ALIGN_CENTER);

    fillFont();
    remix_hud_layer_place_glyphs(&right, &sTestFont, glyphs, TEST_GLYPHS);
    CHECK_EQ(84, (int)glyphs[0].x);
    CHECK_EQ(92, (int)glyphs[1].x);
    remix_hud_layer_place_glyphs(&centre, &sTestFont, glyphs, TEST_GLYPHS);
    CHECK_EQ(92, (int)glyphs[0].x);
    CHECK_EQ(100, (int)glyphs[1].x);
    /* 16 * 0.875 = 14 wide: starts at 153, the second glyph 7 further */
    remix_hud_layer_place_glyphs(&scaled, &sTestFont, glyphs, TEST_GLYPHS);
    CHECK_EQ(153 * 16, sixteenths(glyphs[0].x));
    CHECK_EQ(160 * 16, sixteenths(glyphs[1].x));
    CHECK_EQ(14, sixteenths(glyphs[1].scalex));
    CHECK_EQ(14, sixteenths(glyphs[1].scaley));
}

static void testShortGlyphDropsByTheMissingHeightTimesTheScale(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString string = makeString("Aa", 0.0f, 0.5f, REMIX_HUD_ALIGN_LEFT);

    fillFont();
    remix_hud_layer_place_glyphs(&string, &sTestFont, glyphs, TEST_GLYPHS);
    CHECK_EQ(100, (int)glyphs[0].y);
    CHECK_EQ(102, (int)glyphs[1].y);
}

static void testMaxWidthSqueezesXOnlyAndKeepsTheAnchor(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString string = makeString("ABCD", 100.0f, 1.0f, REMIX_HUD_ALIGN_CENTER);

    fillFont();
    /* fit width 3 * 8 + 16 = 40, squeezed to 20: factor 0.5; aligned width 32 becomes 16 */
    string.max_width = 20.0f;
    CHECK_EQ(4, remix_hud_layer_place_glyphs(&string, &sTestFont, glyphs, TEST_GLYPHS));
    CHECK_EQ(92, (int)glyphs[0].x);
    CHECK_EQ(96, (int)glyphs[1].x);
    CHECK_EQ(104, (int)glyphs[3].x);
    CHECK_EQ(8, sixteenths(glyphs[0].scalex));
    CHECK_EQ(16, sixteenths(glyphs[0].scaley));
    CHECK_EQ(100, (int)glyphs[0].y);
}

static void testSqueezeDoesNotShortenTheDropOfAShortGlyph(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString string = makeString("ABCa", 100.0f, 1.0f, REMIX_HUD_ALIGN_LEFT);

    fillFont();
    string.max_width = 20.0f;
    CHECK_EQ(4, remix_hud_layer_place_glyphs(&string, &sTestFont, glyphs, TEST_GLYPHS));
    CHECK_EQ(8, sixteenths(glyphs[3].scalex));
    CHECK_EQ(104, (int)glyphs[3].y);
}

static void testRectIsTruncatedPartByPartAndCutToTheFrame(void)
{
    RemixHudLayerPixelRect pixels;

    CHECK_EQ(1, remix_hud_layer_rect_pixels(130.5f, 20.9f, 2.9f, 3.5f, &pixels));
    CHECK_EQ(130, pixels.ulx);
    CHECK_EQ(20, pixels.uly);
    CHECK_EQ(132, pixels.lrx);
    CHECK_EQ(23, pixels.lry);

    CHECK_EQ(1, remix_hud_layer_rect_pixels(-4.0f, 230.0f, 10.0f, 20.0f, &pixels));
    CHECK_EQ(0, pixels.ulx);
    CHECK_EQ(6, pixels.lrx);
    CHECK_EQ(240, pixels.lry);

    CHECK_EQ(0, remix_hud_layer_rect_pixels(-20.0f, 10.0f, 10.0f, 10.0f, &pixels));
    CHECK_EQ(0, remix_hud_layer_rect_pixels(330.0f, 10.0f, 10.0f, 10.0f, &pixels));
    CHECK_EQ(0, remix_hud_layer_rect_pixels(10.0f, 10.0f, 0.5f, 10.0f, &pixels));
}

static void testMaxWidthLeavesAStringThatFitsAlone(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString string = makeString("AB", 100.0f, 1.0f, REMIX_HUD_ALIGN_LEFT);

    fillFont();
    string.max_width = 24.0f;
    remix_hud_layer_place_glyphs(&string, &sTestFont, glyphs, TEST_GLYPHS);
    CHECK_EQ(108, (int)glyphs[1].x);
    CHECK_EQ(16, sixteenths(glyphs[1].scalex));
}

static void testGlyphScaleEnlargesTheSpriteAndKeepsThePositions(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString string = makeString("Aa", 40.0f, 0.5f, REMIX_HUD_ALIGN_LEFT);

    fillFont();
    string.glyph_scale = 1.25f;
    remix_hud_layer_place_glyphs(&string, &sTestFont, glyphs, TEST_GLYPHS);
    CHECK_EQ(40, (int)glyphs[0].x);
    CHECK_EQ(44, (int)glyphs[1].x);
    CHECK_EQ(102, (int)glyphs[1].y);
    CHECK_EQ(10, sixteenths(glyphs[0].scalex));
    CHECK_EQ(10, sixteenths(glyphs[0].scaley));
}

static void testGlyphScaleAndSqueezeMultiplyOnX(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString string = makeString("ABCD", 0.0f, 1.0f, REMIX_HUD_ALIGN_LEFT);

    fillFont();
    string.max_width = 20.0f;
    string.glyph_scale = 1.25f;
    remix_hud_layer_place_glyphs(&string, &sTestFont, glyphs, TEST_GLYPHS);
    CHECK_EQ(10, sixteenths(glyphs[0].scalex));
    CHECK_EQ(20, sixteenths(glyphs[0].scaley));
    CHECK_EQ(4, (int)glyphs[1].x);
}

static void testEmptyTextAndFullBufferWriteNoMoreThanTheCapacity(void)
{
    RemixHudLayerGlyph glyphs[TEST_GLYPHS];
    RemixHudLayerString empty = makeString("", 0.0f, 1.0f, REMIX_HUD_ALIGN_CENTER);
    RemixHudLayerString spaces = makeString("   ", 0.0f, 1.0f, REMIX_HUD_ALIGN_RIGHT);
    RemixHudLayerString longer = makeString("ABCDEF", 0.0f, 1.0f, REMIX_HUD_ALIGN_LEFT);

    fillFont();
    CHECK_EQ(0, remix_hud_layer_place_glyphs(&empty, &sTestFont, glyphs, TEST_GLYPHS));
    CHECK_EQ(0, remix_hud_layer_place_glyphs(&spaces, &sTestFont, glyphs, TEST_GLYPHS));
    CHECK_EQ(4, remix_hud_layer_place_glyphs(&longer, &sTestFont, glyphs, 4));
    CHECK_EQ('D', glyphs[3].character);
    CHECK_EQ(0, remix_hud_layer_place_glyphs(&longer, &sTestFont, glyphs, 0));
}

void remixTestHudLayerRules(void)
{
    RUN_TEST(testUnboundSlotHasNoLayer);
    RUN_TEST(testLayersAreNumberedFromOneUpToTheLimit);
    RUN_TEST(testBoundRangeBelongsToItsLayerOnly);
    RUN_TEST(testEachSlotKindHasItsOwnRoutes);
    RUN_TEST(testRefusedBindLeavesEverySlotAsItWas);
    RUN_TEST(testSameLayerMayBindASlotTwiceAndTheLastSlotOfAKind);
    RUN_TEST(testLookupOutsideTheSlotsIsNoLayer);
    RUN_TEST(testClearForgetsLayersAndRoutes);
    RUN_TEST(testPutLivesForTheFrameThatFollowsIt);
    RUN_TEST(testPutLivesOneFrameIntervalOfUpdates);
    RUN_TEST(testPutOfAnEarlierSceneOrAcrossTheCounterWrapIsJudgedByDistance);
    RUN_TEST(testBarFillIsTheWholeUnitsOfTheHeldFraction);
    RUN_TEST(testLeftAlignedGlyphsStartAtTheAnchorAndAdvance);
    RUN_TEST(testSpacesAdvanceByFourAndMakeNoGlyph);
    RUN_TEST(testRightAndCentreAnchorsUseTheAlignedWidth);
    RUN_TEST(testShortGlyphDropsByTheMissingHeightTimesTheScale);
    RUN_TEST(testMaxWidthSqueezesXOnlyAndKeepsTheAnchor);
    RUN_TEST(testMaxWidthLeavesAStringThatFitsAlone);
    RUN_TEST(testSqueezeDoesNotShortenTheDropOfAShortGlyph);
    RUN_TEST(testRectIsTruncatedPartByPartAndCutToTheFrame);
    RUN_TEST(testGlyphScaleEnlargesTheSpriteAndKeepsThePositions);
    RUN_TEST(testGlyphScaleAndSqueezeMultiplyOnX);
    RUN_TEST(testEmptyTextAndFullBufferWriteNoMoreThanTheCapacity);
}
