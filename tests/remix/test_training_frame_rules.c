/**
 * test_training_frame_rules.c - unit tests of the Training D-pad shortcut rules (sc/sc1pmode/sc1ptrainingframerules.c),
 * Training.asm advance_frame_ 1043-1300.
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1ptrainingframerules.h"

#define ON SC1PTRAININGFRAMERULES_DPAD_ON
#define RESET_ONLY SC1PTRAININGFRAMERULES_DPAD_RESET_ONLY
#define OFF SC1PTRAININGFRAMERULES_DPAD_OFF

/** One frame: reads the D-pad, then resolves the frame. */
static SC1PTrainingFrameKind stepFrame(SC1PTrainingFrameState *state, int32_t option, int32_t right, int32_t up, int32_t is_lag_skip)
{
    sc1PTrainingFrameRulesReadDpad(state, option, 0, right, up, 0);

    return sc1PTrainingFrameRulesResolve(state, is_lag_skip);
}

static void testUpTogglesTheFreeze(void)
{
    SC1PTrainingFrameState state;

    sc1PTrainingFrameRulesInit(&state);
    CHECK_EQ(nSC1PTrainingFrameKindRun, stepFrame(&state, ON, 0, 0, 0));
    CHECK_EQ(nSC1PTrainingFrameKindFrozen, stepFrame(&state, ON, 0, 1, 0));
    CHECK_EQ(nSC1PTrainingFrameKindFrozen, stepFrame(&state, ON, 0, 0, 0));
    CHECK_EQ(nSC1PTrainingFrameKindRun, stepFrame(&state, ON, 0, 1, 0));
}

static void testRightFreezesAndAdvancesExactlyOneFrame(void)
{
    SC1PTrainingFrameState state;

    sc1PTrainingFrameRulesInit(&state);
    CHECK_EQ(nSC1PTrainingFrameKindAdvance, stepFrame(&state, ON, 1, 0, 0));
    CHECK_EQ(1, state.is_frozen);
    CHECK_EQ(nSC1PTrainingFrameKindFrozen, stepFrame(&state, ON, 0, 0, 0));
    CHECK_EQ(nSC1PTrainingFrameKindAdvance, stepFrame(&state, ON, 1, 1, 0));
    CHECK_EQ(1, state.is_frozen);
}

static void testALagTicKeepsThePressForTheNextFrame(void)
{
    SC1PTrainingFrameState state;

    sc1PTrainingFrameRulesInit(&state);
    stepFrame(&state, ON, 0, 1, 0);

    CHECK_EQ(nSC1PTrainingFrameKindSkip, stepFrame(&state, ON, 1, 0, 1));
    CHECK_EQ(1, state.is_right_latched);
    CHECK_EQ(nSC1PTrainingFrameKindAdvance, stepFrame(&state, ON, 0, 0, 0));
    CHECK_EQ(0, state.is_right_latched);
    CHECK_EQ(nSC1PTrainingFrameKindSkip, stepFrame(&state, ON, 0, 0, 1));
    CHECK_EQ(nSC1PTrainingFrameKindFrozen, stepFrame(&state, ON, 0, 0, 0));

    CHECK_EQ(nSC1PTrainingFrameKindSkip, stepFrame(&state, ON, 0, 1, 1));
    CHECK_EQ(1, state.is_frozen);
    CHECK_EQ(nSC1PTrainingFrameKindRun, stepFrame(&state, ON, 0, 0, 0));
}

static void testALatchedPressIgnoresNewPresses(void)
{
    SC1PTrainingFrameState state;

    sc1PTrainingFrameRulesInit(&state);
    CHECK_EQ(nSC1PTrainingFrameKindSkip, stepFrame(&state, ON, 0, 1, 1));
    CHECK_EQ(0, sc1PTrainingFrameRulesReadDpad(&state, ON, 0, 1, 0, 1));
    CHECK_EQ(0, state.is_right_latched);
    CHECK_EQ(nSC1PTrainingFrameKindFrozen, sc1PTrainingFrameRulesResolve(&state, 0));
}

static void testResetOnlyAndOffClearTheFreeze(void)
{
    SC1PTrainingFrameState state;

    sc1PTrainingFrameRulesInit(&state);
    stepFrame(&state, ON, 0, 1, 0);
    CHECK_EQ(nSC1PTrainingFrameKindRun, stepFrame(&state, RESET_ONLY, 0, 0, 0));
    CHECK_EQ(0, state.is_frozen);

    stepFrame(&state, ON, 0, 1, 0);
    CHECK_EQ(nSC1PTrainingFrameKindRun, stepFrame(&state, OFF, 0, 0, 0));

    CHECK_EQ(nSC1PTrainingFrameKindRun, stepFrame(&state, RESET_ONLY, 1, 1, 0));
    CHECK_EQ(nSC1PTrainingFrameKindRun, stepFrame(&state, OFF, 1, 1, 0));
}

static void testLeftResetsUnlessOffAndEndsTheRead(void)
{
    SC1PTrainingFrameState state;

    sc1PTrainingFrameRulesInit(&state);
    CHECK_EQ(SC1PTRAININGFRAMERULES_ACTION_RESET, sc1PTrainingFrameRulesReadDpad(&state, ON, 1, 1, 1, 1));
    CHECK_EQ(0, state.is_right_latched);
    CHECK_EQ(0, state.is_up_latched);
    CHECK_EQ(SC1PTRAININGFRAMERULES_ACTION_RESET, sc1PTrainingFrameRulesReadDpad(&state, RESET_ONLY, 1, 0, 0, 0));
    CHECK_EQ(0, sc1PTrainingFrameRulesReadDpad(&state, OFF, 1, 0, 0, 0));
}

static void testDownCyclesTheModelDisplayOnlyWhenOnAndRightIsNotPressed(void)
{
    SC1PTrainingFrameState state;

    sc1PTrainingFrameRulesInit(&state);
    CHECK_EQ(SC1PTRAININGFRAMERULES_ACTION_MODEL_DISPLAY, sc1PTrainingFrameRulesReadDpad(&state, ON, 0, 0, 0, 1));
    CHECK_EQ(0, sc1PTrainingFrameRulesReadDpad(&state, RESET_ONLY, 0, 0, 0, 1));
    CHECK_EQ(0, sc1PTrainingFrameRulesReadDpad(&state, OFF, 0, 0, 0, 1));
    CHECK_EQ(0, sc1PTrainingFrameRulesReadDpad(&state, ON, 0, 1, 0, 1));
}

void remixTestTrainingFrameRules(void)
{
    printf("training D-pad shortcut rules\n");
    RUN_TEST(testUpTogglesTheFreeze);
    RUN_TEST(testRightFreezesAndAdvancesExactlyOneFrame);
    RUN_TEST(testALagTicKeepsThePressForTheNextFrame);
    RUN_TEST(testALatchedPressIgnoresNewPresses);
    RUN_TEST(testResetOnlyAndOffClearTheFreeze);
    RUN_TEST(testLeftResetsUnlessOffAndEndsTheRead);
    RUN_TEST(testDownCyclesTheModelDisplayOnlyWhenOnAndRightIsNotPressed);
}
