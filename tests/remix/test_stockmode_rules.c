/**
 * test_stockmode_rules.c - unit tests of the Stock Mode count rules (gm/gmremixstockmoderules.c).
 */

#include "test_check.h"

#include "gm/gmremixstockmoderules.h"

static void testAnUnsetLastCountGivesTheMatchCount(void)
{
    CHECK_EQ(3, gmRemixStockModeRulesLastCount(-1, 3, 0));
}

static void testTheLastCountIsLoweredToTheMatchCount(void)
{
    CHECK_EQ(1, gmRemixStockModeRulesLastCount(1, 3, 0));
    CHECK_EQ(3, gmRemixStockModeRulesLastCount(5, 3, 0));
}

static void testTugOfWarKeepsALastCountAboveTheMatchCount(void)
{
    CHECK_EQ(5, gmRemixStockModeRulesLastCount(5, 3, 1));
}

static void testTugOfWarStillStartsFromTheMatchCountWhenUnset(void)
{
    CHECK_EQ(3, gmRemixStockModeRulesLastCount(-1, 3, 1));
}

static void testTheManualCountNeverPassesTheMatchCount(void)
{
    CHECK_EQ(1, gmRemixStockModeRulesManualCount(1, 3));
    CHECK_EQ(3, gmRemixStockModeRulesManualCount(7, 3));
}

static void testAnUnsetManualCountIsTheMatchCount(void)
{
    CHECK_EQ(9, gmRemixStockModeRulesManualCount(-1, 9));
    CHECK_EQ(0, gmRemixStockModeRulesManualCount(-1, 0));
}

static void testTheCssCountOfAnUnsetTableIsTheRulesCount(void)
{
    CHECK_EQ(3, gmRemixStockModeRulesCssCount(-1, 3, 0));
    CHECK_EQ(3, gmRemixStockModeRulesCssCount(-1, 3, 1));
}

static void testTheCssCountIsLoweredToTheRulesCountOutsideTugOfWar(void)
{
    CHECK_EQ(1, gmRemixStockModeRulesCssCount(1, 3, 0));
    CHECK_EQ(3, gmRemixStockModeRulesCssCount(3, 3, 0));
    CHECK_EQ(3, gmRemixStockModeRulesCssCount(8, 3, 0));
    CHECK_EQ(8, gmRemixStockModeRulesCssCount(8, 3, 1));
}

static void testAManualArrowStepsTheCountInsideTheRulesCount(void)
{
    CHECK_EQ(2, gmRemixStockModeRulesStepManualCount(1, 1, 3));
    CHECK_EQ(0, gmRemixStockModeRulesStepManualCount(1, -1, 3));
    CHECK_EQ(3, gmRemixStockModeRulesStepManualCount(2, 1, 3));
}

static void testAManualArrowWrapsAtBothEnds(void)
{
    CHECK_EQ(0, gmRemixStockModeRulesStepManualCount(3, 1, 3));
    CHECK_EQ(3, gmRemixStockModeRulesStepManualCount(0, -1, 3));
    CHECK_EQ(0, gmRemixStockModeRulesStepManualCount(7, 1, 3));
}

static void testAManualArrowOnAnUnsetCountStartsFromTheRulesCount(void)
{
    CHECK_EQ(2, gmRemixStockModeRulesStepManualCount(-1, -1, 3));
    CHECK_EQ(0, gmRemixStockModeRulesStepManualCount(-1, 1, 3));
}

void remixTestStockModeRules(void)
{
    printf("stock mode rules\n");
    RUN_TEST(testAnUnsetLastCountGivesTheMatchCount);
    RUN_TEST(testTheLastCountIsLoweredToTheMatchCount);
    RUN_TEST(testTugOfWarKeepsALastCountAboveTheMatchCount);
    RUN_TEST(testTugOfWarStillStartsFromTheMatchCountWhenUnset);
    RUN_TEST(testTheManualCountNeverPassesTheMatchCount);
    RUN_TEST(testAnUnsetManualCountIsTheMatchCount);
    RUN_TEST(testTheCssCountOfAnUnsetTableIsTheRulesCount);
    RUN_TEST(testTheCssCountIsLoweredToTheRulesCountOutsideTugOfWar);
    RUN_TEST(testAManualArrowStepsTheCountInsideTheRulesCount);
    RUN_TEST(testAManualArrowWrapsAtBothEnds);
    RUN_TEST(testAManualArrowOnAnUnsetCountStartsFromTheRulesCount);
}
