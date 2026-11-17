/**
 * test_inputdisplay_rules.c - unit tests of the Input Display rules (if/ifremixinputdisplayrules.c): block placement of
 * create_input_display_objects_ and the stick and gate alphas of update_input_display_.
 */

#include "test_check.h"

#include "if/ifremixinputdisplayrules.h"

static IFRemixInputDisplayLayout resolve(int is_bottom, IFRemixInputDisplayScreen screen, IFRemixInputDisplayMode mode, int stage,
                                         int is_dragon_king)
{
    IFRemixInputDisplayLayoutInput input = { is_bottom, screen, mode, stage, is_dragon_king };
    IFRemixInputDisplayLayout layout;

    ifRemixInputDisplayRulesResolveLayout(&input, &layout);
    return layout;
}

static void testVsKeepsTheRequestedEdgeWithNoOffset(void)
{
    IFRemixInputDisplayLayout top = resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_VS, IFREMIX_INPUTDISPLAY_MODE_OTHER, 0, 0);
    IFRemixInputDisplayLayout bottom = resolve(1, IFREMIX_INPUTDISPLAY_SCREEN_VS, IFREMIX_INPUTDISPLAY_MODE_OTHER, 0, 0);

    CHECK_EQ(0, top.is_bottom);
    CHECK_EQ(0, (int)top.offset_y);
    CHECK_EQ(1, bottom.is_bottom);
    CHECK_EQ(0, (int)bottom.offset_y);
}

static void testTrainingAlwaysGoesToTheBottom(void)
{
    CHECK_EQ(1, resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_TRAINING, IFREMIX_INPUTDISPLAY_MODE_OTHER, 0, 0).is_bottom);
}

static void testBonusTopIsLoweredBy20(void)
{
    CHECK_EQ(20, (int)resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_BONUS, IFREMIX_INPUTDISPLAY_MODE_OTHER, 0, 0).offset_y);
    CHECK_EQ(0, (int)resolve(1, IFREMIX_INPUTDISPLAY_SCREEN_BONUS, IFREMIX_INPUTDISPLAY_MODE_OTHER, 0, 0).offset_y);
}

static void testMultimanTopIsLoweredBy14(void)
{
    CHECK_EQ(14, (int)resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_MULTIMAN, 0, 0).offset_y);
    CHECK_EQ(0, (int)resolve(1, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_MULTIMAN, 0, 0).offset_y);
}

static void testAllStarRestAreaForcesTheBottom(void)
{
    IFRemixInputDisplayLayout layout =
        resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_ALLSTAR, IFREMIX_INPUTDISPLAY_STAGE_REST_AREA, 0);

    CHECK_EQ(1, layout.is_bottom);
    CHECK_EQ(0, (int)layout.offset_y);
}

static void testAllStarYoshiTeamTopIsLoweredBy30(void)
{
    CHECK_EQ(30, (int)resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_ALLSTAR,
                              IFREMIX_INPUTDISPLAY_STAGE_YOSHI_TEAM, 0).offset_y);
    CHECK_EQ(0, (int)resolve(1, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_ALLSTAR,
                             IFREMIX_INPUTDISPLAY_STAGE_YOSHI_TEAM, 0).offset_y);
}

static void testSingleTeamStagesLowerTheTop(void)
{
    CHECK_EQ(20, (int)resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_SINGLE,
                              IFREMIX_INPUTDISPLAY_STAGE_YOSHI_TEAM, 0).offset_y);
    CHECK_EQ(14, (int)resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_SINGLE,
                              IFREMIX_INPUTDISPLAY_STAGE_KIRBY_TEAM, 0).offset_y);
    CHECK_EQ(30, (int)resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_SINGLE,
                              IFREMIX_INPUTDISPLAY_STAGE_POLYGON_TEAM, 0).offset_y);
    CHECK_EQ(0, (int)resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_SINGLE, 3, 0).offset_y);
    CHECK_EQ(0, (int)resolve(1, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_SINGLE,
                             IFREMIX_INPUTDISPLAY_STAGE_YOSHI_TEAM, 0).offset_y);
}

static void testDragonKingHudRestoresTheRequestedEdgeAndOffsets(void)
{
    IFRemixInputDisplayLayout top = resolve(0, IFREMIX_INPUTDISPLAY_SCREEN_TRAINING, IFREMIX_INPUTDISPLAY_MODE_OTHER, 0, 1);
    IFRemixInputDisplayLayout bottom = resolve(1, IFREMIX_INPUTDISPLAY_SCREEN_TRAINING, IFREMIX_INPUTDISPLAY_MODE_OTHER, 0, 1);
    IFRemixInputDisplayLayout rest_area =
        resolve(1, IFREMIX_INPUTDISPLAY_SCREEN_OTHER, IFREMIX_INPUTDISPLAY_MODE_ALLSTAR, IFREMIX_INPUTDISPLAY_STAGE_REST_AREA, 1);

    CHECK_EQ(0, top.is_bottom);
    CHECK_EQ(30, (int)top.offset_y);
    CHECK_EQ(1, bottom.is_bottom);
    CHECK_EQ(14, (int)bottom.offset_y);
    CHECK_EQ(-3, (int)rest_area.offset_y);
}

static void testStickMovesSixHundredthsOfAPixelPerUnitAndClamps(void)
{
    IFRemixInputDisplayStick stick;

    ifRemixInputDisplayRulesUpdateStick(50, 25, 0, &stick);
    CHECK_EQ(3, (int)stick.offset_x);
    CHECK_EQ(-1, (int)stick.offset_y);
    ifRemixInputDisplayRulesUpdateStick(127, -127, 0, &stick);
    CHECK_EQ(4, (int)stick.offset_x);
    CHECK_EQ(4, (int)stick.offset_y);
}

static void testCentredStickAndGateAreDim(void)
{
    IFRemixInputDisplayStick stick;

    ifRemixInputDisplayRulesUpdateStick(0, 0, 0, &stick);
    CHECK_EQ(0x8F, stick.stick_alpha);
    CHECK_EQ(0x4F, stick.gate_alpha);
    CHECK_EQ(0, stick.cooldown);
}

static void testMovedStickLightsBothAndStartsTheCooldown(void)
{
    IFRemixInputDisplayStick stick;

    ifRemixInputDisplayRulesUpdateStick(10, 0, 0, &stick);
    CHECK_EQ(0xFF, stick.stick_alpha);
    CHECK_EQ(0xAF, stick.gate_alpha);
    CHECK_EQ(-6, stick.cooldown);
}

static void testGateStaysLitForSixFramesAfterTheStickReturns(void)
{
    IFRemixInputDisplayStick stick;
    int cooldown = IFREMIX_INPUTDISPLAY_GATE_COOLDOWN;
    int frame;

    for (frame = 0; frame < 6; frame++)
    {
        ifRemixInputDisplayRulesUpdateStick(0, 0, cooldown, &stick);
        CHECK_EQ(0xAF, stick.gate_alpha);
        CHECK_EQ(0xFF, stick.stick_alpha);
        cooldown = stick.cooldown;
    }
    ifRemixInputDisplayRulesUpdateStick(0, 0, cooldown, &stick);
    CHECK_EQ(0x4F, stick.gate_alpha);
    CHECK_EQ(0x8F, stick.stick_alpha);
}

void remixTestInputDisplayRules(void)
{
    printf("input display rules\n");
    RUN_TEST(testVsKeepsTheRequestedEdgeWithNoOffset);
    RUN_TEST(testTrainingAlwaysGoesToTheBottom);
    RUN_TEST(testBonusTopIsLoweredBy20);
    RUN_TEST(testMultimanTopIsLoweredBy14);
    RUN_TEST(testAllStarRestAreaForcesTheBottom);
    RUN_TEST(testAllStarYoshiTeamTopIsLoweredBy30);
    RUN_TEST(testSingleTeamStagesLowerTheTop);
    RUN_TEST(testDragonKingHudRestoresTheRequestedEdgeAndOffsets);
    RUN_TEST(testStickMovesSixHundredthsOfAPixelPerUnitAndClamps);
    RUN_TEST(testCentredStickAndGateAreDim);
    RUN_TEST(testMovedStickLightsBothAndStartsTheCooldown);
    RUN_TEST(testGateStaysLitForSixFramesAfterTheStickReturns);
}
