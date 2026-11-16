/**
 * test_turnip_rules.c - unit tests of the Turnip ground timer halfword (it/itremix/itremixturniprules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixturniprules.h"

static void testGroundTimerLeavesFortyEightFramesOnTheGround(void)
{
    ITRemixTurnipRulesGroundFields fields = itRemixTurnipRulesSplitGroundTimer(ITREMIX_TURNIP_RULES_GROUND_TIMER);

    CHECK_EQ(48, fields.pickup_wait);
    CHECK_TRUE(fields.pickup_wait != ITREMIX_TURNIP_RULES_GROUND_TIMER);
}

static void testGroundTimerSetsTheKnockbackAndUnusedFlags(void)
{
    ITRemixTurnipRulesGroundFields fields = itRemixTurnipRulesSplitGroundTimer(ITREMIX_TURNIP_RULES_GROUND_TIMER);

    CHECK_EQ(1, fields.is_allow_knockback);
    CHECK_EQ(1, fields.is_unused_item_bool);
    CHECK_EQ(0, fields.is_static_damage);
}

static void testSplitMatchesTheMrSaturnHalfword(void)
{
    ITRemixTurnipRulesGroundFields fields = itRemixTurnipRulesSplitGroundTimer(0x5674);

    CHECK_EQ(0x567, fields.pickup_wait);
    CHECK_EQ(0, fields.is_allow_knockback);
    CHECK_EQ(1, fields.is_unused_item_bool);
    CHECK_EQ(0, fields.is_static_damage);
}

static void testEveryFlagBitMapsToItsField(void)
{
    CHECK_EQ(1, itRemixTurnipRulesSplitGroundTimer(0x0008).is_allow_knockback);
    CHECK_EQ(1, itRemixTurnipRulesSplitGroundTimer(0x0004).is_unused_item_bool);
    CHECK_EQ(1, itRemixTurnipRulesSplitGroundTimer(0x0002).is_static_damage);
    CHECK_EQ(0xFFF, itRemixTurnipRulesSplitGroundTimer(0xFFF0).pickup_wait);
    CHECK_EQ(0, itRemixTurnipRulesSplitGroundTimer(0x000F).pickup_wait);
}

void remixTestTurnipRules(void)
{
    RUN_TEST(testGroundTimerLeavesFortyEightFramesOnTheGround);
    RUN_TEST(testGroundTimerSetsTheKnockbackAndUnusedFlags);
    RUN_TEST(testSplitMatchesTheMrSaturnHalfword);
    RUN_TEST(testEveryFlagBitMapsToItsField);
}
