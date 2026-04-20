/**
 *   GPStar Proton Stream Target Trainer
 *   Copyright (C) 2023-2026 GPStar Technologies <contact@gpstartechnologies.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

// Forward function declarations.
void sendInfraredEvent(const char* message); // From Webhandler.h
void notifyWSClients(); // From Webhandler.h

// Extern declarations for global handlers
extern InfraredManager* irManager;

// Helper function to send structured JSON data via IR events.
// Broadcasts real-time infrared activity and target status to web clients via Server-Sent Events.
// All fields are always included to provide explicit data state (empty/null values indicate no data).
void sendInfraredJSON(const char* eventType, const char* deviceType = "", const char* deviceId = "",
                      const char* streamType = "", uint8_t powerLevel = 0, uint16_t damage = 0, const char* message = "") {
  String jsonData;
  JsonDocument jsonBody;

  // Always include all event information for explicit data state
  jsonBody["eventType"] = eventType;  // Type: status_update, ir_received, ir_ignored, target_hit, target_defeated
  jsonBody["status"] = (PSTT_STATUS == PSTT_READY ? "ready" : "disabled");
  jsonBody["health"] = pstt_current_health;
  jsonBody["maxHealth"] = targetConfig.maxHealth;
  jsonBody["healthPercent"] = (pstt_current_health * 100) / targetConfig.maxHealth;
  jsonBody["deviceType"] = deviceType;   // WAND, SINGLESHOT, or empty string
  jsonBody["deviceId"] = deviceId;       // Unique device identifier or empty string
  jsonBody["streamType"] = streamType;   // PROTON, SLIME, STASIS, MESON, or empty string
  jsonBody["powerLevel"] = powerLevel;   // 1-5 for wand power level, or 0 if not applicable
  jsonBody["damage"] = damage;           // Damage amount dealt, or 0 if not applicable
  jsonBody["message"] = message;         // Descriptive debug message or empty string

  // Serialize to JSON string and send via SSE to all connected web clients
  serializeJson(jsonBody, jsonData);
  sendInfraredEvent(jsonData.c_str());
}

void setIndicatorColor(uint8_t i_colour) {
  pstt_jewel_leds[JEWEL_LED_MAX - 1] = getHueColour(i_colour);
}

void setIndicatorLEDs(uint8_t i_colour, uint8_t i_leds_start, uint8_t i_leds_end) {
  for(uint8_t i_tmp = i_leds_start; i_tmp < i_leds_end; i_tmp++) {
    pstt_jewel_leds[i_tmp] = getHueColour(i_colour);
  }
}

void setIndicatorLED(uint8_t i_colour, uint8_t i_led) {
  pstt_jewel_leds[i_led] = getHueColour(i_colour);
}

// Target is ready.
void setTargetAsReady() {
  PSTT_STATUS = PSTT_READY;

  // Reset the target health and generator.
  ms_pstt_health_regen.start(targetConfig.healthRegenDelay);
  pstt_current_health = targetConfig.maxHealth;

  psttServoControl(180);
  setIndicatorLEDs(C_GREEN, (JEWEL_LED_MAX / 2) - (JEWEL_LED_MAX / 2), JEWEL_LED_MAX);
  setIndicatorColor(C_GREEN);

  ms_pstt_blink_rate.start(i_pstt_blink_rate_delay);

  notifyWSClients();
 }

 // The target has been defeated.
void setTargetDefeated() {
  PSTT_STATUS = PSTT_DISABLED;

  psttServoControl(0);
  ms_pstt_health_regen.stop();

  notifyWSClients();
}

// Subtract health from the target.
void reduceTargetHealth(uint16_t i_pwr_subtract) {
  if(pstt_current_health - i_pwr_subtract > PSTT_MIN_HEALTH) {
    pstt_current_health -= i_pwr_subtract;
  }
  else {
    pstt_current_health = PSTT_MIN_HEALTH;
  }

  setIndicatorColor(C_LIGHT_BLUE);
}

// Add health and restart the regeneration timer.
void regenerateTargetHealth() {
  if(pstt_current_health + targetConfig.healthRegen <= targetConfig.maxHealth) {
    pstt_current_health += targetConfig.healthRegen;
  }
  else {
    pstt_current_health = targetConfig.maxHealth;
  }

  ms_pstt_health_regen.start(targetConfig.healthRegenDelay);

  // Send real-time status update after health regeneration
  sendInfraredJSON("status_update", "", "", "", 0, 0, "Health regenerated");
}

// Checks the health and status of the target.
void checkTargetHealth() {
  switch(PSTT_STATUS) {
    case PSTT_DISABLED:
      // Nothing.
    break;

    case PSTT_READY:
    default:
      if(pstt_current_health == PSTT_MIN_HEALTH) {
        setTargetDefeated();
      }
      else {
        if(ms_pstt_health_regen.justFinished()) {
          regenerateTargetHealth();
        }
      }
    break;
  }
}

// Update the LED indicators based on the health of the target.
void updateHealthIndicators() {
  switch(PSTT_STATUS) {
    case PSTT_DISABLED:
      setIndicatorLEDs(C_RED, (JEWEL_LED_MAX / 2) - (JEWEL_LED_MAX / 2), JEWEL_LED_MAX);
      setIndicatorColor(C_RED);
    break;

    case PSTT_READY:
    default:
      if(irManager->isRXTimerExpired()) {
        setIndicatorColor(C_GREEN);
      }

      // Blink the main jewel, and adjust colours if needed.
      if(ms_pstt_blink_rate.justFinished()) {
        ms_pstt_blink_rate.start(i_pstt_blink_rate_delay);

        b_colourChange = !b_colourChange;

        if(pstt_current_health < targetConfig.maxHealth) {
          if(pstt_current_health <= targetConfig.extremeLowHealth) {
            setIndicatorLEDs(b_colourChange ? C_RED : C_BLACK, (JEWEL_LED_MAX / 2) - (JEWEL_LED_MAX / 2), JEWEL_LED_MAX / 2); // All 7 of the jewel LEDs.
          }
          else if(pstt_current_health <= targetConfig.lowHealth) {
            setIndicatorLEDs(b_colourChange ? C_YELLOW : C_BLACK, (JEWEL_LED_MAX / 2) - (JEWEL_LED_MAX / 2), JEWEL_LED_MAX / 2); // All 7 of the jewel LEDs.
          }
          else {
            setIndicatorLEDs(b_colourChange ? C_GREEN : C_BLACK, (JEWEL_LED_MAX / 2) - (JEWEL_LED_MAX / 2), JEWEL_LED_MAX / 2); // All 7 of the jewel LEDs.
          }
        }
        else {
          setIndicatorLEDs(C_GREEN, (JEWEL_LED_MAX / 2) - (JEWEL_LED_MAX / 2), JEWEL_LED_MAX / 2); // All 7 of the jewel LEDs.
        }
      }

      // Change the blink rate of the main jewel depending on the target health.
      // It is better to blink all at once, as it is easier to see at a distance.
      if(pstt_current_health >= 930) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY;
      }
      else if(pstt_current_health >= 860) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 2;
      }
      else if(pstt_current_health >= 790) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 3;
      }
      else if(pstt_current_health >= 720) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 4;
      }
      else if(pstt_current_health >= 650) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 5;
      }
      else if(pstt_current_health >= 580) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 6;
      }
      else if(pstt_current_health >= 510) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 7;
      }
      else if(pstt_current_health >= targetConfig.lowHealth) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 8;
      }
      else if(pstt_current_health >= targetConfig.extremeLowHealth) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 9;
      }
      else if(pstt_current_health >= PSTT_MIN_HEALTH) {
        i_pstt_blink_rate_delay = PSTT_BLINK_RATE_DEFAULT_DELAY / 10;
      }
    break;
  }
}

// Processes incoming infrared signals and sends real-time status updates via SSE.
// Called continuously from main loop for immediate IR response and frequent status broadcasts.
void checkInfraredData() {
  // Send real-time status updates every 1 second for SSE clients
  static unsigned long lastStatusUpdate = 0;
  if(millis() - lastStatusUpdate > 1000) {
    sendInfraredJSON("status_update", "", "", "", 0, 0, "Heartbeat");
    lastStatusUpdate = millis();
  }

  // Exit early if target is not ready
  if(PSTT_STATUS != PSTT_READY) {
    return;
  }

  // Exit early if no IR data to decode
  if(!irManager->decodeData()) {
    return;
  }

  // Translate device type
  const char* deviceName = "UNKNOWN";
  switch(irManager->dataDeviceType()) {
    case IR_DEVICE_NEUTRONA_WAND: deviceName = "WAND"; break;
    case IR_DEVICE_SINGLE_SHOT: deviceName = "SINGLESHOT"; break;
    case IR_DEVICE_PSTT: deviceName = "PSTT"; break;
  }

  // Send raw IR data received event
  String rawMsg = String("Pre=0x") + String(irManager->dataPreamble(), HEX) +
                  " Dev=" + irManager->dataDeviceType() +
                  " Cmd=" + irManager->dataCommand();
  sendInfraredJSON("ir_received", deviceName, String(irManager->getDeviceID()).c_str(), "", 0, 0, rawMsg.c_str());

  // Exit if not NEC-extended protocol
  if(!irManager->dataIsNEC()) {
    sendInfraredJSON("ir_ignored", deviceName, String(irManager->getDeviceID()).c_str(), "", 0, 0, "Not NEC-extended protocol");
    irManager->resumeData();
    return;
  }

  // Exit if wrong preamble
  if(irManager->dataPreamble() != IR_GPSTAR_PREAMBLE) {
    String msg = String("Wrong preamble 0x") + String(irManager->dataPreamble(), HEX);
    sendInfraredJSON("ir_ignored", deviceName, String(irManager->getDeviceID()).c_str(), "", 0, 0, msg.c_str());
    irManager->resumeData();
    return;
  }

  // Exit if unknown device type
  if(irManager->dataDeviceType() != IR_DEVICE_NEUTRONA_WAND && irManager->dataDeviceType() != IR_DEVICE_SINGLE_SHOT) {
    String msg = String("Unknown device type ") + irManager->dataDeviceType();
    sendInfraredJSON("ir_ignored", deviceName, String(irManager->getDeviceID()).c_str(), "", 0, 0, msg.c_str());
    irManager->resumeData();
    return;
  }

  // Process TEST command
  if(irManager->dataCommand() == IR_COMMAND_TEST) {
    ms_pstt_health_regen.start(targetConfig.healthRegenDelay);
    irManager->startRXTimer(500);

    uint16_t damage = targetConfig.wandPower3;
    reduceTargetHealth(damage);

    sendInfraredJSON("target_hit", deviceName, String(irManager->getDeviceID()).c_str(), "TEST", 3, damage, "Test command processed");
    irManager->resumeData();
    return;
  }

  // Parse command into stream type and power level
  uint8_t streamType = 0; // 0=UNKNOWN, 1=PROTON, 2=SLIME, 3=STASIS, 4=MESON, 5=BOSON DART, 6=SLIME BLAST, 7=SHOCK BLAST, 8=OVERLOAD PULSE, 9=CROSSING THE STREAMS
  uint8_t powerLevel = 0; // 0-4 (for power levels 1-5)
  const char* streamName = "UNKNOWN";

  if(irManager->dataCommand() >= IR_COMMAND_PROTON && irManager->dataCommand() <= IR_COMMAND_PROTON + 4) {
    streamType = 1;
    streamName = "PROTON";
    powerLevel = irManager->dataCommand() - IR_COMMAND_PROTON;
  }
  else if(irManager->dataCommand() >= IR_COMMAND_BOSON_DART && irManager->dataCommand() <= IR_COMMAND_BOSON_DART + 4) {
    streamType = 5;
    streamName = "BOSON DART";
    powerLevel = irManager->dataCommand() - IR_COMMAND_BOSON_DART;
  }
  else if(irManager->dataCommand() >= IR_COMMAND_SLIME && irManager->dataCommand() <= IR_COMMAND_SLIME + 4) {
    streamType = 2;
    streamName = "SLIME";
    powerLevel = irManager->dataCommand() - IR_COMMAND_SLIME;
  }
  else if(irManager->dataCommand() >= IR_COMMAND_SLIME_BLAST && irManager->dataCommand() <= IR_COMMAND_SLIME_BLAST + 4) {
    streamType = 6;
    streamName = "SLIME BLAST";
    powerLevel = irManager->dataCommand() - IR_COMMAND_SLIME_BLAST;
  }
  else if(irManager->dataCommand() >= IR_COMMAND_STASIS && irManager->dataCommand() <= IR_COMMAND_STASIS + 4) {
    streamType = 3;
    streamName = "STASIS";
    powerLevel = irManager->dataCommand() - IR_COMMAND_STASIS;
  }
  else if(irManager->dataCommand() >= IR_COMMAND_SHOCK_BLAST && irManager->dataCommand() <= IR_COMMAND_SHOCK_BLAST + 4) {
    streamType = 7;
    streamName = "SHOCK BLAST";
    powerLevel = irManager->dataCommand() - IR_COMMAND_SHOCK_BLAST;
  }
  else if(irManager->dataCommand() >= IR_COMMAND_MESON && irManager->dataCommand() <= IR_COMMAND_MESON + 4) {
    streamType = 4;
    streamName = "MESON";
    powerLevel = irManager->dataCommand() - IR_COMMAND_MESON;
  }
  else if(irManager->dataCommand() >= IR_COMMAND_OVERLOAD_PULSE && irManager->dataCommand() <= IR_COMMAND_OVERLOAD_PULSE + 4) {
    streamType = 8;
    streamName = "OVERLOAD PULSE";
    powerLevel = irManager->dataCommand() - IR_COMMAND_OVERLOAD_PULSE;
  }
  else if(irManager->dataCommand() >= IR_COMMAND_CTS && irManager->dataCommand() <= IR_COMMAND_CTS + 4) {
    streamType = 9;
    streamName = "CROSSING THE STREAMS";
    powerLevel = irManager->dataCommand() - IR_COMMAND_CTS;
  }

  // Exit if unknown stream type
  if(streamType == 0) {
    String msg = String("Unknown command ") + irManager->dataCommand();
    sendInfraredJSON("ir_ignored", deviceName, String(irManager->getDeviceID()).c_str(), "", 0, 0, msg.c_str());
    irManager->resumeData();
    return;
  }

  // Handle SingleShot instant defeat (only for PROTON stream)
  if(irManager->dataDeviceType() == IR_DEVICE_SINGLE_SHOT && streamType == 1) {
    pstt_current_health = PSTT_MIN_HEALTH;
    sendInfraredJSON("target_defeated", deviceName, String(irManager->getDeviceID()).c_str(), streamName, powerLevel + 1, targetConfig.maxHealth, "Instant defeat by SingleShot");
    irManager->resumeData();
    return;
  }

  // Process Neutrona Wand firing
  if(irManager->dataDeviceType() == IR_DEVICE_NEUTRONA_WAND) {
    ms_pstt_health_regen.start(targetConfig.healthRegenDelay);
    irManager->startRXTimer(500);

    // Calculate damage based on power level
    uint16_t damage = 0;

    switch(streamType) {
      // Proton, Slime, Stasis, Meson
      case 1 ... 4:
      default:
        switch(powerLevel) {
          case 0: damage = targetConfig.wandPower1; break;
          case 1: damage = targetConfig.wandPower2; break;
          case 2: damage = targetConfig.wandPower3; break;
          case 3: damage = targetConfig.wandPower4; break;
          case 4:
          default: damage = targetConfig.wandPower5; break;
        }
      break;

      case 5:
        // Boson Dart
        // This should make the pack overheat faster.
        switch(powerLevel) {
          case 0: damage = PSTT_MAX_HEALTH / 5; break;
          case 1: damage = PSTT_MAX_HEALTH / 4; break;
          case 2: damage = PSTT_MAX_HEALTH / 3; break;
          case 3: damage = PSTT_MAX_HEALTH / 2; break;
          case 4:
          default: damage = PSTT_MAX_HEALTH; break;
        }
      break;

       case 6:
          // Slime Blast
          switch(powerLevel) {
            case 0: damage = PSTT_MAX_HEALTH / 20; break;
            case 1: damage = PSTT_MAX_HEALTH / 16; break;
            case 2: damage = PSTT_MAX_HEALTH / 12; break;
            case 3: damage = PSTT_MAX_HEALTH / 8; break;
            case 4:
            default: damage = PSTT_MAX_HEALTH / 4; break;
          }
        break;

        case 7:
          // Shock Blast
          // If we can detect how far away the target is from the shooter, then make the damage weaker. Shock blast is suppose to be useless at long range.
          switch(powerLevel) {
            case 0: damage = PSTT_MAX_HEALTH / 10; break;
            case 1: damage = PSTT_MAX_HEALTH / 8; break;
            case 2: damage = PSTT_MAX_HEALTH / 6; break;
            case 3: damage = PSTT_MAX_HEALTH / 4; break;
            case 4:
            default: damage = PSTT_MAX_HEALTH / 2; break;
          }
        break;

        case 8:
          // Overload Pulse
          switch(powerLevel) {
            case 0: damage = PSTT_MAX_HEALTH / 15; break;
            case 1: damage = PSTT_MAX_HEALTH / 12; break;
            case 2: damage = PSTT_MAX_HEALTH / 9; break;
            case 3: damage = PSTT_MAX_HEALTH / 6; break;
            case 4:
            default: damage = PSTT_MAX_HEALTH / 3; break;
          }
        break;

        case 9:
          // Crossing the streams
          switch(powerLevel) {
            case 0: damage = targetConfig.wandPower1 * 2; break;
            case 1: damage = targetConfig.wandPower2 * 3; break;
            case 2: damage = targetConfig.wandPower3 * 4; break;
            case 3: damage = targetConfig.wandPower4 * 5; break;
            case 4:
            default: damage = targetConfig.wandPower5 * 6; break;
          }
        break;
    }

    reduceTargetHealth(damage);

    // Check if target was defeated
    if(pstt_current_health == PSTT_MIN_HEALTH) {
      sendInfraredJSON("target_defeated", deviceName, String(irManager->getDeviceID()).c_str(), streamName, powerLevel + 1, damage, "Target defeated");
    } else {
      sendInfraredJSON("target_hit", deviceName, String(irManager->getDeviceID()).c_str(), streamName, powerLevel + 1, damage, "Target hit");
    }
  }

  irManager->resumeData();
}