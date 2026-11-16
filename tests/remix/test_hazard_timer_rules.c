/**
 * test_hazard_timer_rules.c - unit tests of the hazard timer rules (gr/grremix/grremixhazardtimerrules.c): the Rainbow
 * Road start sequence and chomp growl window, the Metallic Madness shrink timer and the Spawned Fear Cacodemon factory.
 */

#include "test_check.h"

#include "gr/grremix/grremixhazardtimerrules.h"

static void testRainbowSfxPlaysFiveLoopCuesBetweenTheFirstAndTheLast(void)
{
    int32_t timer;
    int32_t loop_cues = 0;

    for (timer = 0; timer < 0x1000; timer++)
    {
        if (grRemixRainbowRulesSfxCue(timer) == GRREMIX_RAINBOW_FGM_SFX_LOOP)
        {
            loop_cues++;
        }
    }
    CHECK_EQ(5, loop_cues);
}

static void testRainbowSfxCueTimes(void)
{
    CHECK_EQ(GRREMIX_RAINBOW_FGM_SFX_FIRST, grRemixRainbowRulesSfxCue(0x960));
    CHECK_EQ(GRREMIX_RAINBOW_FGM_SFX_LOOP, grRemixRainbowRulesSfxCue(0x99C));
    CHECK_EQ(GRREMIX_RAINBOW_FGM_SFX_LOOP, grRemixRainbowRulesSfxCue(0xA8C));
    CHECK_EQ(GRREMIX_RAINBOW_FGM_SFX_LAST, grRemixRainbowRulesSfxCue(0xAC8));
    CHECK_EQ(GRREMIX_RAINBOW_FGM_SFX_NONE, grRemixRainbowRulesSfxCue(0x95F));
    CHECK_EQ(GRREMIX_RAINBOW_FGM_SFX_NONE, grRemixRainbowRulesSfxCue(0xA8D));
    CHECK_EQ(GRREMIX_RAINBOW_FGM_SFX_NONE, grRemixRainbowRulesSfxCue(0xAC9));
}

static void testRainbowGrowlWindowFollowsTheLowByteOfTheDrawFrame(void)
{
    CHECK_EQ(1, grRemixRainbowRulesGrowlWindowIsOpen(0));
    CHECK_EQ(1, grRemixRainbowRulesGrowlWindowIsOpen(14));
    CHECK_EQ(0, grRemixRainbowRulesGrowlWindowIsOpen(15));
    CHECK_EQ(0, grRemixRainbowRulesGrowlWindowIsOpen(255));
    CHECK_EQ(1, grRemixRainbowRulesGrowlWindowIsOpen((uint8_t)(256 * 5 + 3)));
}

static void testMetallicMushroomClearsTheShrinkTimer(void)
{
    CHECK_EQ(0, grRemixMetallicRulesShrinkTimerKept(900, 1));
    CHECK_EQ(0, grRemixMetallicRulesShrinkTimerKept(1, 1));
    CHECK_EQ(900, grRemixMetallicRulesShrinkTimerKept(900, 0));
    CHECK_EQ(0, grRemixMetallicRulesShrinkTimerKept(0, 0));
}

static void testCacodemonFactoryWaitsWithoutRollHitOrForcedFrame(void)
{
    CHECK_EQ(nGRRemixCacodemonFactoryActionNone, grRemixCacodemonRulesFactoryAction(1500, 79, 0));
    CHECK_EQ(nGRRemixCacodemonFactoryActionNone, grRemixCacodemonRulesFactoryAction(2999, 0, 1));
    CHECK_EQ(nGRRemixCacodemonFactoryActionNone, grRemixCacodemonRulesFactoryAction(3001, 1499, 0));
}

static void testCacodemonFactorySpawnsWhenNoneIsAlive(void)
{
    CHECK_EQ(nGRRemixCacodemonFactoryActionSpawn, grRemixCacodemonRulesFactoryAction(1700, GRREMIX_CACODEMON_ROLL_HIT, 0));
    CHECK_EQ(nGRRemixCacodemonFactoryActionSpawn, grRemixCacodemonRulesFactoryAction(3000, 0, 0));
}

static void testCacodemonFactoryRestartsTheTimerWhenOneIsAlive(void)
{
    CHECK_EQ(nGRRemixCacodemonFactoryActionRestart, grRemixCacodemonRulesFactoryAction(1700, GRREMIX_CACODEMON_ROLL_HIT, 1));
    CHECK_EQ(nGRRemixCacodemonFactoryActionRestart, grRemixCacodemonRulesFactoryAction(3000, 0, 1));
}

void remixTestHazardTimerRules(void)
{
    RUN_TEST(testRainbowSfxPlaysFiveLoopCuesBetweenTheFirstAndTheLast);
    RUN_TEST(testRainbowSfxCueTimes);
    RUN_TEST(testRainbowGrowlWindowFollowsTheLowByteOfTheDrawFrame);
    RUN_TEST(testMetallicMushroomClearsTheShrinkTimer);
    RUN_TEST(testCacodemonFactoryWaitsWithoutRollHitOrForcedFrame);
    RUN_TEST(testCacodemonFactorySpawnsWhenNoneIsAlive);
    RUN_TEST(testCacodemonFactoryRestartsTheTimerWhenOneIsAlive);
}
