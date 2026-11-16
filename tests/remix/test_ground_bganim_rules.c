/**
 * test_ground_bganim_rules.c - unit tests of the Remix background sprite tables (ef/efremixgroundrules.c): which stages
 * run the ground actor (Stages.asm bg_animation_ and add_bg_animation) and the transcribed bg_info rows.
 */

#include "test_check.h"

#include "ef/efremixgroundrules.h"
#include "gr/grremix/grremixids.h"

#include <stddef.h>

/* Stages.asm 5078-5265: one add_bg_animation call per added stage. */
#define TEST_BGANIM_STAGE_COUNT 27

static void testVanillaSlotsAreTheFirstEightStages(void)
{
    CHECK_TRUE(efRemixGroundRulesIsVanillaSlot(nGRRemixStage_PEACHS_CASTLE));
    CHECK_TRUE(efRemixGroundRulesIsVanillaSlot(nGRRemixStage_SAFFRON_CITY));
    CHECK_EQ(0, efRemixGroundRulesIsVanillaSlot(nGRRemixStage_MUSHROOM_KINGDOM));
    CHECK_EQ(0, efRemixGroundRulesIsVanillaSlot(-1));
}

static void testAddedStagesWithBgAnimationAreFound(void)
{
    CHECK_EQ(TEST_BGANIM_STAGE_COUNT, efRemixGroundRulesGetStageCount());
    CHECK_TRUE(efRemixGroundRulesFindStage(nGRRemixStage_SMASHVILLE2) != NULL);
    CHECK_TRUE(efRemixGroundRulesFindStage(nGRRemixStage_GHZ) != NULL);
    CHECK_TRUE(efRemixGroundRulesFindStage(nGRRemixStage_BIG_BOOS_HAUNT) != NULL);
    CHECK_TRUE(efRemixGroundRulesFindStage(nGRRemixStage_SMASHVILLE_REMIX) != NULL);
}

static void testStagesWithoutBgAnimationAreNotFound(void)
{
    CHECK_TRUE(efRemixGroundRulesFindStage(nGRRemixStage_MUSHROOM_KINGDOM) == NULL);
    CHECK_TRUE(efRemixGroundRulesFindStage(nGRRemixStage_MADMM) == NULL);
    CHECK_TRUE(efRemixGroundRulesFindStage(nGRRemixStage_ONETT) == NULL);
    CHECK_TRUE(efRemixGroundRulesFindStage(nGRRemixStage_PEACHS_CASTLE) == NULL);
}

static void testBigBoosHauntRowMatchesStagesBbhAsm(void)
{
    const EFRemixGroundStage *stage = efRemixGroundRulesFindStage(nGRRemixStage_BIG_BOOS_HAUNT);

    CHECK_EQ(0x55D8, stage->o_data);
    CHECK_EQ(1, stage->params_num);
    CHECK_EQ(0, stage->params[0].effect_id);
    CHECK_EQ(1, stage->params[0].make_queue);
    CHECK_EQ(0, stage->params[0].lr);
    CHECK_EQ(1, stage->params[0].effect_weight);
    CHECK_EQ(1, stage->sprites_num);
    CHECK_EQ(0x7258, stage->sprites[0].o_dobjsetup);
    CHECK_EQ(0x7C14, stage->sprites[0].o_matanim_joint);
}

static void testSectorZRemixRowUsesTheVanillaSectorFileOffsets(void)
{
    const EFRemixGroundStage *stage = efRemixGroundRulesFindStage(nGRRemixStage_SECTOR_Z_REMIX);

    CHECK_EQ(0x8680, stage->o_data);
    CHECK_EQ(nEFRemixGroundSpriteProcStep, stage->sprites[0].proc);
    CHECK_EQ(0xAD00, stage->sprites[0].o_dobjsetup);
    CHECK_EQ(0xB6F8, stage->sprites[3].o_dobjsetup);
    CHECK_EQ(0xB7E0, stage->sprites[4].o_anim_joint);
}

static void testAsmDirectionFifteenKeepsTheVanillaDirection(void)
{
    const EFRemixGroundStage *sector = efRemixGroundRulesFindStage(nGRRemixStage_SECTOR_Z_DL);
    const EFRemixGroundStage *yoshi = efRemixGroundRulesFindStage(nGRRemixStage_YOSHI_ISLAND_O);

    CHECK_EQ(-1, sector->params[0].lr);
    CHECK_EQ(+1, sector->params[1].lr);
    CHECK_EQ(-1, sector->params[5].lr);
    CHECK_EQ(-3, yoshi->params[2].lr);
    CHECK_EQ(+3, yoshi->params[3].lr);
}

static void testEveryParamNamesASpriteOfItsStage(void)
{
    int32_t i;
    int32_t j;

    for (i = 0; i < efRemixGroundRulesGetStageCount(); i++)
    {
        const EFRemixGroundStage *stage = efRemixGroundRulesGetStage(i);

        for (j = 0; j < stage->params_num; j++)
        {
            int32_t lr = stage->params[j].lr;

            CHECK_TRUE(stage->params[j].effect_id < stage->sprites_num);
            CHECK_TRUE(stage->params[j].effect_weight != 0);
            CHECK_TRUE((lr == 0) || (lr == -1) || (lr == +1) || (lr == -3) || (lr == +3));
        }
    }
    CHECK_TRUE(efRemixGroundRulesGetStage(TEST_BGANIM_STAGE_COUNT) == NULL);
}

void remixTestGroundBgAnimRules(void)
{
    RUN_TEST(testVanillaSlotsAreTheFirstEightStages);
    RUN_TEST(testAddedStagesWithBgAnimationAreFound);
    RUN_TEST(testStagesWithoutBgAnimationAreNotFound);
    RUN_TEST(testBigBoosHauntRowMatchesStagesBbhAsm);
    RUN_TEST(testSectorZRemixRowUsesTheVanillaSectorFileOffsets);
    RUN_TEST(testAsmDirectionFifteenKeepsTheVanillaDirection);
    RUN_TEST(testEveryParamNamesASpriteOfItsStage);
}
