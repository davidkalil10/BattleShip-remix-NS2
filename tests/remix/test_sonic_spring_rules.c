/**
 * test_sonic_spring_rules.c - unit tests of the shared action check of Sonic's spring (ft/ftremix/sonic/ftremixsonicspringrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/sonic/ftremixsonicspringrules.h"

static void testJumpToPassChangesTheAction(void)
{
    CHECK_EQ(nFTRemixSonicSpringRulesChangeAction, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_JUMPF));
    CHECK_EQ(nFTRemixSonicSpringRulesChangeAction, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_PASS));
    CHECK_EQ(nFTRemixSonicSpringRulesByCharacter, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_JUMPF - 1));
    CHECK_EQ(nFTRemixSonicSpringRulesByCharacter, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_PASS + 1));
}

/* Follows the code of SonicSpecial.asm 1749-1751, not its comment at 1736; see M14 C3 */
static void testOnlyTumbleChangesTheActionNotFallSpecial(void)
{
    CHECK_EQ(nFTRemixSonicSpringRulesChangeAction, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_TUMBLE));
    CHECK_EQ(nFTRemixSonicSpringRulesByCharacter, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_FALLSPECIAL));
}

static void testShieldBreakChangesTheActionAndInhalePulledIsIgnored(void)
{
    CHECK_EQ(nFTRemixSonicSpringRulesChangeAction, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_SHIELDBREAK));
    CHECK_EQ(nFTRemixSonicSpringRulesChangeAction, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_SHIELDBREAKFALL));
    CHECK_EQ(nFTRemixSonicSpringRulesIgnore, ftRemixSonicSpringRulesCommonAction(FTREMIX_SONIC_SPRING_ACTION_INHALEPULLED));
}

void remixTestSonicSpringRules(void)
{
    printf("sonic spring rules\n");
    RUN_TEST(testJumpToPassChangesTheAction);
    RUN_TEST(testOnlyTumbleChangesTheActionNotFallSpecial);
    RUN_TEST(testShieldBreakChangesTheActionAndInhalePulledIsIgnored);
}
