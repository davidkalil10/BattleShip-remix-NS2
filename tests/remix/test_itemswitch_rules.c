/**
 * test_itemswitch_rules.c - unit tests of the Item Switch extended mask rules (it/itremix/itremixspawnrules.c, Item.asm save_)
 * and of the Turnip despawn guard (it/itremix/itremixturniprules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixspawnrules.h"
#include "it/itremix/itremixturniprules.h"

static void testOneExtendedRowForThirteenStandardItems(void)
{
    CHECK_EQ(1, itRemixSpawnRulesExtendedRowsMask(13, 12));
    CHECK_EQ(7, itRemixSpawnRulesExtendedRowsMask(15, 12));
}

static void testAllRowsOffClearsTheStopwatchBit(void)
{
    CHECK_EQ(0, itRemixSpawnRulesExtendedTogglesAllRowsOff(1, 1));
}

static void testAllRowsOffKeepsBitsOutsideTheRows(void)
{
    CHECK_EQ(0x10, itRemixSpawnRulesExtendedTogglesAllRowsOff(0x11, 1));
}

static void testDespawnGuardClearsTheLowThrowBit(void)
{
    CHECK_EQ(2, itRemixTurnipRulesClearDespawnGuard(3));
    CHECK_EQ(0, itRemixTurnipRulesClearDespawnGuard(1));
    CHECK_EQ(6, itRemixTurnipRulesClearDespawnGuard(7));
    CHECK_EQ(4, itRemixTurnipRulesClearDespawnGuard(4));
}

void remixTestItemSwitchRules(void)
{
    RUN_TEST(testOneExtendedRowForThirteenStandardItems);
    RUN_TEST(testAllRowsOffClearsTheStopwatchBit);
    RUN_TEST(testAllRowsOffKeepsBitsOutsideTheRows);
    RUN_TEST(testDespawnGuardClearsTheLowThrowBit);
}
