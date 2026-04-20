/**
 * Test suite for the standard device state in SUPER_HERO (default).
 */

#include <gtest/gtest.h>
#include "DeviceState.h"
#include <set>

// Test fixture for DeviceState tests that require systemMode to be SUPER_HERO
class DeviceStateDefaultFixture : public ::testing::Test {
protected:
    // This member variable is available in every test using this fixture.
    // It is default-constructed when the fixture is created for each test.
    DeviceState state;

    // SetUp() is called before each test.
    // Use this to set up any common initialization, if needed.
    void SetUp() override {
    }
};

TEST_F(DeviceStateDefaultFixture, CanInstantiate) {
    SUCCEED(); // If we reach this point, instantiation succeeded.
}

// Confirms the expected defaults are set for this mode.
TEST_F(DeviceStateDefaultFixture, ConstructorDefaults) {
    EXPECT_EQ(state.getSystemMode(), MODE_SUPER_HERO);
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_ON); // Always reported ON for Super Hero mode.
    EXPECT_EQ(state.getSystemTheme(), SYSTEM_AFTERLIFE);
    EXPECT_FALSE(state.isTheme80s());
    EXPECT_TRUE(state.isThemeModern());
    EXPECT_EQ(state.getStreamMode(), PROTON);
    EXPECT_EQ(state.getPreviousStreamMode(), PROTON);
    EXPECT_EQ(state.getStreamModeOpts(), (FLAG_STASIS | FLAG_SLIME | FLAG_MESON | FLAG_SPECTRAL | FLAG_HOLIDAY_HALLOWEEN | FLAG_HOLIDAY_CHRISTMAS | FLAG_SPECTRAL_CUSTOM));
    EXPECT_EQ(state.getPowerLevel(), LEVEL_5);
    EXPECT_EQ(state.getFiringMode(), FLAG_VG_MODE);
    EXPECT_EQ(state.getBarrelState(), BARREL_UNKNOWN);
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_NEVER);
}

// Get human-readable names for common properties.
TEST_F(DeviceStateDefaultFixture, GetCommonNames) {
    EXPECT_STREQ(state.getModeName(), "Super Hero");
    EXPECT_STREQ(state.getIonArmSwitchState(), "Ready");
    EXPECT_STREQ(state.getThemeName(), "Afterlife");
    EXPECT_STREQ(state.getStreamModeName(), "Proton Stream");
    EXPECT_STREQ(state.getPowerLevelName(), "5");
    EXPECT_STREQ(state.getBarrelStateName(), "Unknown");
}

// Directly confirm stream flags are available using individual checks.
TEST_F(DeviceStateDefaultFixture, HasStreamFlag) {
    EXPECT_TRUE(state.hasStreamFlag(FLAG_PROTON));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SPECTRAL));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SPECTRAL_CUSTOM));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_HOLIDAY_HALLOWEEN));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_HOLIDAY_CHRISTMAS));
    EXPECT_TRUE(state.supportsVGStreams());
}

// Ensure that all available stream modes are supported.
// Indirectly tests getRequiredStreamFlag() as well.
TEST_F(DeviceStateDefaultFixture, SupportsStreamMode_Defaults) {
    EXPECT_TRUE(state.supportsStreamMode(PROTON));
    EXPECT_TRUE(state.supportsStreamMode(STASIS));
    EXPECT_TRUE(state.supportsStreamMode(SLIME));
    EXPECT_TRUE(state.supportsStreamMode(MESON));
    EXPECT_TRUE(state.supportsStreamMode(SPECTRAL));
    EXPECT_TRUE(state.supportsStreamMode(SPECTRAL_CUSTOM));
    EXPECT_TRUE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));
    EXPECT_TRUE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));
    EXPECT_TRUE(state.supportsVGStreams());
}

// Ensure only specific stream modes are supported.
TEST_F(DeviceStateDefaultFixture, SupportsStreamMode_Proton) {
    state.clearStreamFlags(); // Clear all flags.
    EXPECT_TRUE(state.supportsStreamMode(PROTON));
    EXPECT_FALSE(state.supportsStreamMode(STASIS));
    EXPECT_FALSE(state.supportsStreamMode(SLIME));
    EXPECT_FALSE(state.supportsStreamMode(MESON));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));
    EXPECT_FALSE(state.supportsVGStreams());
}

// Ensure only specific stream modes are supported.
TEST_F(DeviceStateDefaultFixture, SupportsStreamMode_VGOnly) {
    state.clearStreamFlags(); // Clear all flags.
    state.enableVGStreams(); // Only VG modes.
    EXPECT_TRUE(state.supportsStreamMode(PROTON));
    EXPECT_TRUE(state.supportsStreamMode(STASIS));
    EXPECT_TRUE(state.supportsStreamMode(SLIME));
    EXPECT_TRUE(state.supportsStreamMode(MESON));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));
    EXPECT_TRUE(state.supportsVGStreams());
}

// Ensure only specific stream modes are supported.
TEST_F(DeviceStateDefaultFixture, SupportsStreamMode_Spectral) {
    state.clearStreamFlags(); // Clear all flags.
    state.enableSpectralStream(); // Only Spectral mode.
    EXPECT_TRUE(state.supportsStreamMode(PROTON));
    EXPECT_FALSE(state.supportsStreamMode(STASIS));
    EXPECT_FALSE(state.supportsStreamMode(SLIME));
    EXPECT_FALSE(state.supportsStreamMode(MESON));
    EXPECT_TRUE(state.supportsStreamMode(SPECTRAL));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));
    EXPECT_FALSE(state.supportsVGStreams());
}

// Ensure only specific stream modes are supported.
TEST_F(DeviceStateDefaultFixture, SupportsStreamMode_SpectralCustom) {
    state.clearStreamFlags(); // Clear all flags.
    state.enableSpectralCustomStream(); // Only Custom mode.
    EXPECT_TRUE(state.supportsStreamMode(PROTON));
    EXPECT_FALSE(state.supportsStreamMode(STASIS));
    EXPECT_FALSE(state.supportsStreamMode(SLIME));
    EXPECT_FALSE(state.supportsStreamMode(MESON));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL));
    EXPECT_TRUE(state.supportsStreamMode(SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));
    EXPECT_FALSE(state.supportsVGStreams());
}

// Ensure only specific stream modes are supported.
TEST_F(DeviceStateDefaultFixture, SupportsStreamMode_Holidays) {
    state.clearStreamFlags(); // Clear all flags.
    EXPECT_TRUE(state.supportsStreamMode(PROTON));
    EXPECT_FALSE(state.supportsStreamMode(STASIS));
    EXPECT_FALSE(state.supportsStreamMode(SLIME));
    EXPECT_FALSE(state.supportsStreamMode(MESON));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));
    EXPECT_FALSE(state.supportsVGStreams());

    state.enableHalloweenStream(); // Add Halloween mode.
    EXPECT_TRUE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));

    state.enableChristmasStream(); // Add Christmas mode.
    EXPECT_TRUE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));
}

TEST_F(DeviceStateDefaultFixture, SetStreamMode_Default) {
    // Default is PROTON so we should be in the expected mode.
    EXPECT_TRUE(state.inStreamMode(PROTON));
    EXPECT_FALSE(state.setStreamMode(PROTON));
    EXPECT_EQ(state.getStreamMode(), PROTON);

    // Clear stream options and test again
    state.clearStreamFlags(); // Clear all flags.
    EXPECT_FALSE(state.setStreamMode(SPECTRAL));

    // Restore flag and try again
    state.enableSpectralStream(); // Only Spectral mode.
    EXPECT_TRUE(state.setStreamMode(SPECTRAL));
    EXPECT_EQ(state.getStreamMode(), SPECTRAL);
}

TEST_F(DeviceStateDefaultFixture, SetStreamMode_Settings) {
    // Start stream mode with a non-default value
    EXPECT_FALSE(state.inStreamMode(SETTINGS));
    EXPECT_TRUE(state.setStreamMode(SETTINGS));
    EXPECT_TRUE(state.inStreamMode(SETTINGS));
    EXPECT_EQ(state.getStreamMode(), SETTINGS);
    EXPECT_EQ(state.getPreviousStreamMode(), PROTON);
    EXPECT_TRUE(state.switchedFromStream(PROTON));

    // Clear stream options and test again
    state.clearStreamFlags(); // Clear all flags.
    EXPECT_FALSE(state.setStreamMode(SPECTRAL));

    // Restore flag and try again
    state.enableSpectralStream(); // Only Spectral mode.
    EXPECT_TRUE(state.setStreamMode(SPECTRAL));
    EXPECT_EQ(state.getStreamMode(), SPECTRAL);
}

// Negative test: setting an unsupported mode should not change streamMode
TEST_F(DeviceStateDefaultFixture, SetStreamMode_NoSpectralSupport) {
    state.clearStreamFlags(); // Clear all flags.
    state.enableVGStreams(); // Only VG modes supported.
    // Try to set SPECTRAL, which is not supported.
    bool result = state.setStreamMode(SPECTRAL);
    EXPECT_FALSE(result);
    // streamMode should remain unchanged
    EXPECT_EQ(state.getStreamMode(), PROTON);
}

// Negative test: setting an unsupported mode should not change streamMode
TEST_F(DeviceStateDefaultFixture, SetStreamMode_NoVGSupport) {
    state.clearStreamFlags(); // Clear all flags.
    // Try to set SLIME, which is not supported.
    bool result = state.setStreamMode(SLIME);
    EXPECT_FALSE(result);
    // streamMode should remain unchanged
    EXPECT_EQ(state.getStreamMode(), PROTON);
}

// Test nextStreamMode cycles through all modes in an expected order.
TEST_F(DeviceStateDefaultFixture, NextStreamModeCyclesAllModes) {
    // Vector to record the order in which stream modes are cycled
    std::vector<uint8_t> cycled;
    cycled.push_back(state.getStreamMode());
    // Cycle through enough times to visit all modes (assuming 8 modes)
    for (int i = 0; i < 8; ++i) {
        state.setStreamMode(state.nextStreamMode());
        cycled.push_back(state.getStreamMode());
    }
    // Expected order based on enum definition and cycling logic
    std::vector<uint8_t> expected = {
        PROTON, STASIS, SLIME, MESON, SPECTRAL, HOLIDAY_HALLOWEEN, HOLIDAY_CHRISTMAS, SPECTRAL_CUSTOM, PROTON
    };
    // Check that the cycled order matches the expected order
    EXPECT_EQ(cycled, expected);
}

// Test previousStreamMode cycles through all modes in an expected order.
TEST_F(DeviceStateDefaultFixture, PreviousStreamModeCyclesAllModes) {
    // Vector to record the order in which stream modes are cycled backwards
    std::vector<uint8_t> cycled;
    cycled.push_back(state.getStreamMode());
    // Cycle through enough times to visit all modes (assuming 8 modes)
    for (int i = 0; i < 8; ++i) {
        state.setStreamMode(state.previousStreamMode());
        cycled.push_back(state.getStreamMode());
    }
    // Expected order when cycling backwards from PROTON
    std::vector<uint8_t> expected = {
        PROTON, SPECTRAL_CUSTOM, HOLIDAY_CHRISTMAS, HOLIDAY_HALLOWEEN, SPECTRAL, MESON, SLIME, STASIS, PROTON
    };
    // Check that the cycled order matches the expected reverse order
    EXPECT_EQ(cycled, expected);
}

// Test all firing mode setters and checks
TEST_F(DeviceStateDefaultFixture, FiringModeChecks) {
    state.setFiringModeVG();
    EXPECT_TRUE(state.isFiringModeVG());
    state.setFiringModeCTS();
    EXPECT_TRUE(state.isFiringModeCTS());
    state.setFiringModeCTSMix();
    EXPECT_TRUE(state.isFiringModeCTSMix());
}

// Test red switch mode getter/setter
TEST_F(DeviceStateDefaultFixture, IonArmSwitch) {
    // Default in SUPER_HERO always reports ON
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_ON);

    // Changing red switch mode should have no effect in SUPER_HERO
    EXPECT_TRUE(state.setIonArmSwitch(RED_SWITCH_OFF));
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_ON);
}

// Test vibration mode getter/setter
TEST_F(DeviceStateDefaultFixture, VibrationMode) {
    // Default is VIBRATION_NEVER
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_NEVER);

    // Set to another mode
    EXPECT_TRUE(state.setVibrationMode(VIBRATION_ALWAYS));
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_ALWAYS);

    // Set to a different mode
    EXPECT_TRUE(state.setVibrationMode(VIBRATION_FIRING_ONLY));
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_FIRING_ONLY);
}

// Test exporting DeviceState to WandSyncData
TEST_F(DeviceStateDefaultFixture, ExportToWandSyncData) {
    // Set up DeviceState with specific values
    state.setSystemTheme(SYSTEM_1984);
    state.setStreamMode(SPECTRAL);
    state.setPowerLevel(LEVEL_3);
    state.setVibrationMode(VIBRATION_ALWAYS); // Mode changed, but won't be exported.

    // Create sync data and export
    WandSyncData syncData;
    state.exportData(syncData);

    // Verify exported values
    EXPECT_EQ(syncData.systemMode, MODE_SUPER_HERO);
    EXPECT_EQ(syncData.systemTheme, SYSTEM_1984);
    EXPECT_EQ(syncData.streamMode, SPECTRAL);
    EXPECT_TRUE(syncData.ionArmSwitch); // Should be true for RED_SWITCH_ON
    EXPECT_EQ(syncData.powerLevel, LEVEL_3);
    EXPECT_FALSE(syncData.vibrationToggle); // Expect default on export, regardless of mode ENUM
}

// Test importing AttenuatorSyncData to DeviceState
TEST_F(DeviceStateDefaultFixture, ImportFromAttenuatorSyncData) {
    // Create sync data with specific values
    AttenuatorSyncData syncData;
    syncData.systemMode = MODE_SUPER_HERO;
    syncData.systemTheme = SYSTEM_AFTERLIFE;
    syncData.streamFlags = FLAG_MESON;
    syncData.streamMode = MESON;
    syncData.ionArmSwitch = true;
    syncData.powerLevel = LEVEL_4;
    syncData.barrelExtended = false;

    // Import into DeviceState
    state.importData(syncData);

    // Verify imported values
    EXPECT_EQ(state.getSystemMode(), MODE_SUPER_HERO);
    EXPECT_EQ(state.getSystemTheme(), SYSTEM_AFTERLIFE);
    EXPECT_EQ(state.getStreamModeOpts(), FLAG_MESON);
    EXPECT_EQ(state.getStreamMode(), MESON);
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_ON); // Should convert true to RED_SWITCH_ON
    EXPECT_EQ(state.getPowerLevel(), LEVEL_4);
    EXPECT_EQ(state.getBarrelState(), BARREL_RETRACTED); // Should convert false to BARREL_RETRACTED
}

// Test round-trip: export -> import should preserve values
TEST_F(DeviceStateDefaultFixture, RoundTripWandSync) {
    // Set up initial state
    state.setSystemTheme(SYSTEM_1984);
    state.setStreamMode(SLIME);
    state.setPowerLevel(LEVEL_3);
    state.setVibrationMode(VIBRATION_FIRING_ONLY); // Mode changed, but won't be exported.

    // Export to sync data
    WandSyncData syncData;
    state.exportData(syncData);

    // Create a new DeviceState and import
    DeviceState newState;
    newState.importData(syncData);

    // Values should match (considering type conversions)
    EXPECT_EQ(newState.getSystemMode(), MODE_SUPER_HERO);
    EXPECT_EQ(newState.getSystemTheme(), SYSTEM_1984);
    EXPECT_EQ(newState.getStreamMode(), SLIME);
    EXPECT_EQ(newState.getIonArmSwitch(), RED_SWITCH_ON);
    EXPECT_EQ(newState.getPowerLevel(), LEVEL_3);
    EXPECT_EQ(newState.getVibrationMode(), VIBRATION_NEVER); // Expect the default for the state.
}

// Test individual enable/disable methods for VG streams
TEST_F(DeviceStateDefaultFixture, EnableDisableStasisStream) {
    state.clearStreamFlags(); // Start with no flags.
    EXPECT_FALSE(state.hasStreamFlag(FLAG_STASIS));

    state.enableStasisStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_TRUE(state.supportsStreamMode(STASIS));

    state.disableStasisStream();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_FALSE(state.supportsStreamMode(STASIS));
}

TEST_F(DeviceStateDefaultFixture, EnableDisableSlimeStream) {
    state.clearStreamFlags();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SLIME));

    state.enableSlimeStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_TRUE(state.supportsStreamMode(SLIME));

    state.disableSlimeStream();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_FALSE(state.supportsStreamMode(SLIME));
}

TEST_F(DeviceStateDefaultFixture, EnableDisableMesonStream) {
    state.clearStreamFlags();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_MESON));

    state.enableMesonStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_TRUE(state.supportsStreamMode(MESON));

    state.disableMesonStream();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_FALSE(state.supportsStreamMode(MESON));
}

// Test group enable/disable for VG streams
TEST_F(DeviceStateDefaultFixture, EnableDisableVGStreams) {
    state.clearStreamFlags();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_FALSE(state.supportsVGStreams());

    state.enableVGStreams();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_TRUE(state.supportsVGStreams());

    state.disableVGStreams();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_FALSE(state.supportsVGStreams());
}

// Test individual enable/disable methods for spectral streams
TEST_F(DeviceStateDefaultFixture, EnableDisableSpectralStream) {
    state.clearStreamFlags();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL));

    state.enableSpectralStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SPECTRAL));
    EXPECT_TRUE(state.supportsStreamMode(SPECTRAL));

    state.disableSpectralStream();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL));
}

TEST_F(DeviceStateDefaultFixture, EnableDisableSpectralCustomStream) {
    state.clearStreamFlags();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL_CUSTOM));

    state.enableSpectralCustomStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SPECTRAL_CUSTOM));
    EXPECT_TRUE(state.supportsStreamMode(SPECTRAL_CUSTOM));

    state.disableSpectralCustomStream();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.supportsStreamMode(SPECTRAL_CUSTOM));
}

// Test individual enable/disable methods for holiday streams
TEST_F(DeviceStateDefaultFixture, EnableDisableHalloweenStream) {
    state.clearStreamFlags();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_HOLIDAY_HALLOWEEN));

    state.enableHalloweenStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_HOLIDAY_HALLOWEEN));
    EXPECT_TRUE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));

    state.disableHalloweenStream();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_HALLOWEEN));
}

TEST_F(DeviceStateDefaultFixture, EnableDisableChristmasStream) {
    state.clearStreamFlags();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_HOLIDAY_CHRISTMAS));

    state.enableChristmasStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_HOLIDAY_CHRISTMAS));
    EXPECT_TRUE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));

    state.disableChristmasStream();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_HOLIDAY_CHRISTMAS));
    EXPECT_FALSE(state.supportsStreamMode(HOLIDAY_CHRISTMAS));
}

// Test group enable/disable for all spectral and holiday streams
TEST_F(DeviceStateDefaultFixture, EnableDisableAllSpectralStreams) {
    state.clearStreamFlags();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_HOLIDAY_CHRISTMAS));
    EXPECT_FALSE(state.supportsSpectralStreams());

    state.enableAllSpectralStreams();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SPECTRAL));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SPECTRAL_CUSTOM));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_HOLIDAY_HALLOWEEN));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_HOLIDAY_CHRISTMAS));
    EXPECT_TRUE(state.supportsSpectralStreams());

    state.removeAllSpectralStreams();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_HOLIDAY_CHRISTMAS));
    EXPECT_FALSE(state.supportsSpectralStreams());
}

// Test that disable methods work without VG mode guard (can always disable)
TEST_F(DeviceStateDefaultFixture, DisableWorksWithoutVGMode) {
    // Enable a flag first
    state.enableStasisStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_STASIS));

    // Switch to CTS mode (not VG mode)
    state.setFiringModeCTS();
    EXPECT_FALSE(state.isFiringModeVG());

    // Disable should still work even without VG mode
    state.disableStasisStream();
    EXPECT_FALSE(state.hasStreamFlag(FLAG_STASIS));
}

// Test that VG streams are preserved when disabling individual spectral streams
TEST_F(DeviceStateDefaultFixture, DisableSpectralPreservesVGStreams) {
    state.clearStreamFlags();
    state.enableVGStreams(); // Enable VG streams
    state.enableSpectralStream(); // Add spectral

    EXPECT_TRUE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SPECTRAL));

    // Disabling spectral should not affect VG streams
    state.disableSpectralStream();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_FALSE(state.hasStreamFlag(FLAG_SPECTRAL));
    EXPECT_TRUE(state.supportsVGStreams());
}

// Test that removeAllSpectralStreams preserves VG streams
TEST_F(DeviceStateDefaultFixture, RemoveSpectralPreservesVGStreams) {
    state.clearStreamFlags();
    state.enableVGStreams();
    state.enableAllSpectralStreams();

    EXPECT_TRUE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_TRUE(state.supportsSpectralStreams());

    // Remove all spectral streams but keep VG streams
    state.removeAllSpectralStreams();
    EXPECT_TRUE(state.hasStreamFlag(FLAG_STASIS));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_SLIME));
    EXPECT_TRUE(state.hasStreamFlag(FLAG_MESON));
    EXPECT_FALSE(state.supportsSpectralStreams());
    EXPECT_TRUE(state.supportsVGStreams());
}
