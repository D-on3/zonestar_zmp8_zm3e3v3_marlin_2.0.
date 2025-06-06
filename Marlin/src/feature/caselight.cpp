/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(CASE_LIGHT_ENABLE)

#include "caselight.h"

CaseLight caselight;

uint8_t CaseLight::brightness = CASE_LIGHT_DEFAULT_BRIGHTNESS;
bool CaseLight::on = CASE_LIGHT_DEFAULT_ON;

#if ENABLED(CASE_LIGHT_USE_NEOPIXEL)
  LEDColor CaseLight::color =
    #ifdef CASE_LIGHT_NEOPIXEL_COLOR
      CASE_LIGHT_NEOPIXEL_COLOR
    #else
      { 255, 255, 255, 255 }
    #endif
  ;
#endif

#ifndef INVERT_CASE_LIGHT
  #define INVERT_CASE_LIGHT false
#endif

void CaseLight::update(const bool sflag) {
  /**
   * The brightness_sav (and sflag) is needed because ARM chips ignore
   * a "WRITE(CASE_LIGHT_PIN,x)" command to the pins that are directly
   * controlled by the PWM module. In order to turn them off the brightness
   * level needs to be set to OFF. Since we can't use the PWM register to
   * save the last brightness level we need a variable to save it.
   */
  static uint8_t brightness_sav;  // Save brightness info for restore on "M355 S1"

  if (on || !sflag)
    brightness_sav = brightness;  // Save brightness except for M355 S0
  if (sflag && on)
    brightness = brightness_sav;  // Restore last brightness for M355 S1

  #if ENABLED(CASE_LIGHT_USE_NEOPIXEL) || DISABLED(CASE_LIGHT_NO_BRIGHTNESS)
    const uint8_t i = on ? brightness : 0, n10ct = INVERT_CASE_LIGHT ? 255 - i : i;
  #endif

  #if ENABLED(CASE_LIGHT_USE_NEOPIXEL)

    leds.set_color(
      MakeLEDColor(color.r, color.g, color.b, color.w, n10ct),
      false
    );

  #else // !CASE_LIGHT_USE_NEOPIXEL

    #if DISABLED(CASE_LIGHT_NO_BRIGHTNESS)
      if (PWM_PIN(CASE_LIGHT_PIN))
        analogWrite(pin_t(CASE_LIGHT_PIN), (
          #if CASE_LIGHT_MAX_PWM == 255
            n10ct
          #else
            map(n10ct, 0, 255, 0, CASE_LIGHT_MAX_PWM)
          #endif
        ));
      else
    #endif
      {
        const bool s = on ? !INVERT_CASE_LIGHT : INVERT_CASE_LIGHT;
        WRITE(CASE_LIGHT_PIN, s ? HIGH : LOW);
      }
  #endif // !CASE_LIGHT_USE_NEOPIXEL
}

#if ENABLED(CASE_LIGHT_SMART)
void CaseLight::update_brightness_BLN(const uint8_t brightness) {
  if (!on) return;
  const uint8_t n10ct = INVERT_CASE_LIGHT ? 255 - brightness : brightness;
  if (PWM_PIN(CASE_LIGHT_PIN))
    analogWrite(pin_t(CASE_LIGHT_PIN), (
      #if CASE_LIGHT_MAX_PWM == 255
        n10ct
      #else
        map(n10ct, 0, 255, 0, CASE_LIGHT_MAX_PWM)
      #endif
    ));
  else
  {
    const bool s = on ? !INVERT_CASE_LIGHT : INVERT_CASE_LIGHT;
    WRITE(CASE_LIGHT_PIN, s ? HIGH : LOW);
  }
}
#endif

#endif // CASE_LIGHT_ENABLE
