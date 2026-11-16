/**
 * test_zebes_acid_rules.c - unit tests of the lava levels of the Zebes-based stages (gr/grremix/grremixzebes.c).
 */

#include "test_check.h"

#include <stdint.h>

#include "gr/grremix/grremixids.h"
#include "gr/grremix/grremixzebes.h"

#define TEST_ZEBES_GKIND_PLANET_ZEBES 3

static const float sTestVanillaLevels[GRREMIX_ZEBES_ACID_LEVELS_NUM] =
{
    -3600.0F, -1000.0F, -200.0F, 1800.0F, -3600.0F, 2600.0F, -1000.0F, -500.0F,
    -400.0F, 800.0F, 1200.0F, -1000.0F, 0.0F, -1000.0F, 500.0F, -3000.0F
};

/** Writes the levels the stage selects into the table, as grZebesMakeAcid does at every match start. */
static void testApplyStageLevels(int32_t gkind, float *table)
{
    const float *levels = grRemixZebesSelectAcidLevels(gkind, sTestVanillaLevels);
    int32_t i;

    for (i = 0; i < GRREMIX_ZEBES_ACID_LEVELS_NUM; i++)
    {
        table[i] = levels[i];
    }
}

static void testRemixStagesUseTheirOwnLevels(void)
{
    CHECK_TRUE(grRemixZebesSelectAcidLevels(nGRRemixStage_NORFAIR, sTestVanillaLevels) == grRemixGetAltAcidLevels(nGRRemixStage_NORFAIR));
    CHECK_TRUE(grRemixZebesSelectAcidLevels(nGRRemixStage_ZEBES_DL, sTestVanillaLevels) == grRemixGetAltAcidLevels(nGRRemixStage_ZEBES_DL));
}

static void testOtherStagesGetTheVanillaLevels(void)
{
    CHECK_TRUE(grRemixZebesSelectAcidLevels(TEST_ZEBES_GKIND_PLANET_ZEBES, sTestVanillaLevels) == sTestVanillaLevels);
}

static void testVanillaZebesAfterNorfairGetsTheVanillaLevelsBack(void)
{
    float table[GRREMIX_ZEBES_ACID_LEVELS_NUM];
    int32_t i;

    testApplyStageLevels(nGRRemixStage_NORFAIR, table);
    CHECK_TRUE(table[0] == 200.0F);
    testApplyStageLevels(TEST_ZEBES_GKIND_PLANET_ZEBES, table);

    for (i = 0; i < GRREMIX_ZEBES_ACID_LEVELS_NUM; i++)
    {
        CHECK_TRUE(table[i] == sTestVanillaLevels[i]);
    }
}

void remixTestZebesAcidRules(void)
{
    printf("zebes acid rules\n");
    RUN_TEST(testRemixStagesUseTheirOwnLevels);
    RUN_TEST(testOtherStagesGetTheVanillaLevels);
    RUN_TEST(testVanillaZebesAfterNorfairGetsTheVanillaLevelsBack);
}
