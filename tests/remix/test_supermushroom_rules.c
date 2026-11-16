/**
 * test_supermushroom_rules.c - unit tests of the mushroom size effect rules (it/itremix/itremixsupermushroomrules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixsupermushroomrules.h"

static void testPoisonEffectGetsTheFrameBeforeItsGrowBack(void)
{
    int32_t poison_transition2 = ITREMIX_SUPERMUSHROOM_RULES_SHRINK_FRAMES + ITREMIX_SUPERMUSHROOM_RULES_MAINTAIN_FRAMES;

    CHECK_EQ(542, itRemixSuperMushroomRulesGetSupersededTimer(0));
    CHECK_EQ(poison_transition2 - 1, itRemixSuperMushroomRulesGetSupersededTimer(0));
}

static void testSuperEffectGetsTheFrameBeforeItsShrinkBack(void)
{
    int32_t super_transition2 = ITREMIX_SUPERMUSHROOM_RULES_GROW_FRAMES + ITREMIX_SUPERMUSHROOM_RULES_MAINTAIN_FRAMES;

    CHECK_EQ(551, itRemixSuperMushroomRulesGetSupersededTimer(1));
    CHECK_EQ(super_transition2 - 1, itRemixSuperMushroomRulesGetSupersededTimer(1));
}

void remixTestSuperMushroomRules(void)
{
    RUN_TEST(testPoisonEffectGetsTheFrameBeforeItsGrowBack);
    RUN_TEST(testSuperEffectGetsTheFrameBeforeItsShrinkBack);
}
