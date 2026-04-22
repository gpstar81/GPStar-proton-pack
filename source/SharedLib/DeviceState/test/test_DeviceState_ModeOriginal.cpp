/**
 * Test suite for the standard device state in MODE_ORIGINAL mode.
 */

#include <gtest/gtest.h>
#include "DeviceState.h"
#include <set>

// Test fixture for DeviceState tests that require systemMode to be MODE_ORIGINAL
class DeviceStateOriginalFixture : public ::testing::Test {
protected:
    // This member variable is available in every test using this fixture.
    // It is default-constructed when the fixture is created for each test.
    DeviceState state;

    // SetUp() is called before each test.
    // Sets the system mode to MODE_ORIGINAL, ensuring a customized starting state.
    void SetUp() override {
        state.setSystemMode(MODE_ORIGINAL);
    }
};

TEST_F(DeviceStateOriginalFixture, CanInstantiate) {
    SUCCEED(); // If we reach this point, instantiation succeeded.
}

// Confirms the expected defaults are set for this mode.
TEST_F(DeviceStateOriginalFixture, ConstructorDefaults) {
    EXPECT_EQ(state.getSystemMode(), MODE_ORIGINAL); // Confirms change via setSystemMode()
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_OFF);
    EXPECT_EQ(state.getSystemTheme(), SYSTEM_AFTERLIFE);
    EXPECT_FALSE(state.isTheme80s());
    EXPECT_TRUE(state.isThemeModern());
    EXPECT_EQ(state.getStreamMode(), PROTON);
    EXPECT_EQ(state.getPreviousStreamMode(), PROTON);
    EXPECT_TRUE(state.hasStreamFlag(FLAG_PROTON));
    EXPECT_EQ(state.getPowerLevel(), LEVEL_5);
    EXPECT_EQ(state.getFiringMode(), FLAG_CTS_MODE);
    EXPECT_EQ(state.getBarrelState(), BARREL_UNKNOWN);
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_NEVER);
}

// Get human-readable names for common properties.
TEST_F(DeviceStateOriginalFixture, GetCommonNames) {
    EXPECT_STREQ(state.getModeName(), "Original");
    EXPECT_STREQ(state.getIonArmSwitchState(), "Standby");
    EXPECT_STREQ(state.getThemeName(), "Afterlife");
    EXPECT_STREQ(state.getStreamModeName(), "Proton Stream");
    EXPECT_STREQ(state.getPowerLevelName(), "5");
    EXPECT_STREQ(state.getBarrelStateName(), "Unknown");
}

// Ensure that only the PROTON stream mode is supported.
// Indirectly tests getRequiredStreamFlag() as well.
TEST_F(DeviceStateOriginalFixture, SupportsStreamMode) {
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

// Attempts to add other stream mode flags should fail.
TEST_F(DeviceStateOriginalFixture, SupportsStreamMode_Disallowed) {
    state.enableVGStreams(); // Attempt to enable all VG modes.
    state.enableSpectralStream(); // Attempt to enable Spectral mode.
    state.enableSpectralCustomStream(); // Attempt to enable Spectral Custom mode.
    state.enableHalloweenStream(); // Attempt to enable Halloween mode.
    state.enableChristmasStream(); // Attempt to enable Christmas mode.
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

// Attempts to switch to any stream mode should fail.
TEST_F(DeviceStateOriginalFixture, SetStreamMode_Disallowed) {
    EXPECT_FALSE(state.setStreamMode(PROTON)); // We should already be in Proton.
    EXPECT_FALSE(state.setStreamMode(STASIS));
    EXPECT_FALSE(state.setStreamMode(SLIME));
    EXPECT_FALSE(state.setStreamMode(MESON));
    EXPECT_FALSE(state.setStreamMode(SPECTRAL));
    EXPECT_FALSE(state.setStreamMode(SPECTRAL_CUSTOM));
    EXPECT_FALSE(state.setStreamMode(HOLIDAY_HALLOWEEN));
    EXPECT_FALSE(state.setStreamMode(HOLIDAY_CHRISTMAS));
}

// Test red switch mode getter/setter
TEST_F(DeviceStateOriginalFixture, IonArmSwitch) {
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_OFF);
    EXPECT_TRUE(state.setIonArmSwitch(RED_SWITCH_ON));
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_ON);
    EXPECT_TRUE(state.setIonArmSwitch(RED_SWITCH_OFF));
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_OFF);
}

// Test vibration mode getter/setter
TEST_F(DeviceStateOriginalFixture, VibrationMode) {
    // Default is VIBRATION_NEVER
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_NEVER);

    // Set to another mode
    EXPECT_TRUE(state.setVibrationMode(VIBRATION_ALWAYS));
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_ALWAYS);

    // Set to a different mode
    EXPECT_TRUE(state.setVibrationMode(VIBRATION_FIRING_ONLY));
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_FIRING_ONLY);
}

// Test exporting DeviceState to AttenuatorSyncData (MODE_ORIGINAL)
TEST_F(DeviceStateOriginalFixture, ExportToAttenuatorSyncData) {
    // Set up DeviceState with specific values for MODE_ORIGINAL
    state.setSystemTheme(SYSTEM_FROZEN_EMPIRE);
    state.setStreamMode(PROTON); // Only PROTON supported in MODE_ORIGINAL
    state.setIonArmSwitch(RED_SWITCH_OFF);
    state.setPowerLevel(LEVEL_1);
    state.setBarrelState(BARREL_EXTENDED);

    // Create sync data and export
    AttenuatorSyncData syncData;
    state.exportData(syncData);

    // Verify exported values
    EXPECT_EQ(syncData.systemMode, MODE_ORIGINAL);
    EXPECT_EQ(syncData.systemTheme, SYSTEM_FROZEN_EMPIRE);
    EXPECT_EQ(syncData.streamMode, PROTON);
    EXPECT_FALSE(syncData.ionArmSwitch); // Should be false for RED_SWITCH_OFF
    EXPECT_EQ(syncData.powerLevel, LEVEL_1);
    EXPECT_TRUE(syncData.barrelExtended); // Should be true for BARREL_EXTENDED
}

// Test importing WandSyncData to DeviceState (MODE_ORIGINAL)
TEST_F(DeviceStateOriginalFixture, ImportFromWandSyncData) {
    // Create sync data with MODE_ORIGINAL values
    WandSyncData syncData;
    syncData.systemMode = MODE_ORIGINAL;
    syncData.systemTheme = SYSTEM_1989;
    syncData.streamMode = PROTON;
    syncData.ionArmSwitch = false;
    syncData.powerLevel = LEVEL_2;
    syncData.vibrationToggle = true;

    // Import into DeviceState
    state.importData(syncData);

    // Verify imported values
    EXPECT_EQ(state.getSystemMode(), MODE_ORIGINAL);
    EXPECT_EQ(state.getSystemTheme(), SYSTEM_1989);
    EXPECT_EQ(state.getStreamMode(), PROTON);
    EXPECT_EQ(state.getIonArmSwitch(), RED_SWITCH_OFF); // Should convert false to RED_SWITCH_OFF
    EXPECT_EQ(state.getPowerLevel(), LEVEL_2);
    EXPECT_EQ(state.getVibrationMode(), VIBRATION_NEVER); // Expect the default for the state
}

// Test round-trip: export -> import should preserve values (MODE_ORIGINAL)
TEST_F(DeviceStateOriginalFixture, RoundTripAttenuatorSync) {
    // Set up initial state for MODE_ORIGINAL
    state.setSystemTheme(SYSTEM_FROZEN_EMPIRE);
    state.setStreamMode(PROTON); // Only PROTON supported in MODE_ORIGINAL
    state.setIonArmSwitch(RED_SWITCH_OFF);
    state.setPowerLevel(LEVEL_5);
    state.setBarrelState(BARREL_EXTENDED);

    // Export to sync data
    AttenuatorSyncData syncData;
    state.exportData(syncData);

    // Create a new DeviceState, set to MODE_ORIGINAL, and import
    DeviceState newState;
    newState.setSystemMode(MODE_ORIGINAL);
    newState.importData(syncData);

    // Values should match
    EXPECT_EQ(newState.getSystemMode(), MODE_ORIGINAL);
    EXPECT_EQ(newState.getSystemTheme(), SYSTEM_FROZEN_EMPIRE);
    EXPECT_EQ(newState.getStreamMode(), PROTON);
    EXPECT_EQ(newState.getIonArmSwitch(), RED_SWITCH_OFF);
    EXPECT_EQ(newState.getPowerLevel(), LEVEL_5);
    EXPECT_TRUE(newState.hasStreamFlag(FLAG_PROTON));
    EXPECT_EQ(newState.getBarrelState(), BARREL_EXTENDED);
}
