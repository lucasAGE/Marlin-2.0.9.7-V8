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

/**
 * gcode/temp/M140_M190.cpp
 *
 * Bed target temperature control
 */

#include "../../inc/MarlinConfig.h"

#if HAS_HEATED_BED

  #include "../gcode.h"
  #include "../../module/temperature.h"
  #include "../../lcd/marlinui.h"

  /**
   * M140 - Set Bed Temperature target and return immediately
   * M190 - Set Bed Temperature target and wait
   *
   *  I<index>  : Preset index (if material presets are defined)
   *  S<target> : The target temperature in current units
   *
   * Parameters
   *  I<index>  : Preset index (if material presets are defined)
   *  S<target> : The target temperature in current units. Wait for heating only.
   *
   * M190 Parameters
   *  R<target> : The target temperature in current units. Wait for heating and cooling.
   *
   * Examples
   *  M140 S60 : Set target to 60° and return right away.
   *  M190 R40 : Set target to 40°. Wait until the bed gets close to 40°.
   *
   * With PRINTJOB_TIMER_AUTOSTART turning on heaters will start the print job timer
   *  (used by printingIsActive, etc.) and turning off heaters will stop the timer.
  */

  // Auto-reset para cama 0
  static bool bed_status_reset_bed0() {
    const celsius_t target = thermalManager.degTargetBed(0);
    return target < 30 || thermalManager.degBedNear(0, target);
  }
  // Auto-reset para cama 1
  static bool bed_status_reset_bed1() {
    const celsius_t target = thermalManager.degTargetBed(1);
    return target < 30 || thermalManager.degBedNear(1, target);
  }
  // Auto-reset para cama 2
  static bool bed_status_reset_bed2() {
    const celsius_t target = thermalManager.degTargetBed(2);
    return target < 30 || thermalManager.degBedNear(2, target);
  }
  // Auto-reset para cama 3
  static bool bed_status_reset_bed3() {
    const celsius_t target = thermalManager.degTargetBed(3);
    return target < 30 || thermalManager.degBedNear(3, target);
  }

  static bool bed_status_reset_all() {
    // Retorna true quando TODAS as beds estiverem próximas do target
    for (uint8_t b = 0; b < BED_COUNT; ++b) {
      if (!thermalManager.degBedNear(b, thermalManager.degTargetBed(b)))
        return false;
    }
    return true;
  }
  
  #if HAS_MULTI_BEDS
    void GcodeSuite::M140_M190(const bool isM190) {

      if (DEBUGGING(DRYRUN)) return;

      bool got_temp = false;
      celsius_t temp = 0;

      // 1) Preset via I<index> (mantém igual)
      #if HAS_PREHEAT
        got_temp = parser.seenval('I');
        if (got_temp) {
          const uint8_t index = parser.value_byte();
          temp = ui.material_preset[_MIN(index, PREHEAT_COUNT - 1)].bed_temp;
        }
      #endif

      // 2) Parâmetro S (sem esperar resfriar) ou R (espera sempre)
      bool no_wait_for_cooling = false;
      if (!got_temp) {
        no_wait_for_cooling = parser.seenval('S');
        got_temp = no_wait_for_cooling || (isM190 && parser.seenval('R'));
        if (got_temp) temp = parser.value_celsius();
      }
      if (!got_temp) return;

      // 3) Parâmetro opcional B<n> para cama específica
      bool has_bed_index = parser.seenval('B');
      uint8_t bed_index = has_bed_index ? parser.value_byte() : 0;
      // Valide o índice
      if (has_bed_index && bed_index >= BED_COUNT) {
        SERIAL_ECHOLNPGM("Error: Bed index out of range");
        return;
      }

      // 4) Definir alvo
      if (has_bed_index) {
        thermalManager.setTargetBed(bed_index, temp);
      } else {
        thermalManager.setAllTargetBed(temp);
      }

      // 5) Mensagem no LCD
      if (has_bed_index) {
        // Uma cama específica
        if ( thermalManager.isHeatingBed(bed_index) ) {
          // “Aquecendo mesa N…”
          switch (bed_index) {
            case 0: LCD_MESSAGE(MSG_BED_HEATING1); break;
            case 1: LCD_MESSAGE(MSG_BED_HEATING2); break;
            case 2: LCD_MESSAGE(MSG_BED_HEATING3); break;
            case 3: LCD_MESSAGE(MSG_BED_HEATING4); break;
          }
        }
        else {
          // “Esfriando mesa N…”
          switch (bed_index) {
            case 0: LCD_MESSAGE(MSG_BED_COOLING1); break;
            case 1: LCD_MESSAGE(MSG_BED_COOLING2); break;
            case 2: LCD_MESSAGE(MSG_BED_COOLING3); break;
            case 3: LCD_MESSAGE(MSG_BED_COOLING4); break;
          }
        }
      }
      else {
        // Todas as mesas
        LCD_MESSAGE(MSG_ALL_BEDS_HEATING);
      }

      // 6) Timer de impressão
      TERN_(PRINTJOB_TIMER_AUTOSTART,
        thermalManager.auto_job_check_timer(isM190, !isM190)
      );

      // 7) Espera (M190) ou retorno imediato (M140)
      if (isM190) {
        if (has_bed_index)
          thermalManager.wait_for_bed(bed_index, no_wait_for_cooling);
        else
          thermalManager.wait_for_all_beds(no_wait_for_cooling);
      } else {
        // M140: auto-reset de status quando atingir a temperatura
        if (has_bed_index) {
          // Cama específica
          switch (bed_index) {
            case 0: ui.set_status_reset_fn(bed_status_reset_bed0); break;
            case 1: ui.set_status_reset_fn(bed_status_reset_bed1); break;
            case 2: ui.set_status_reset_fn(bed_status_reset_bed2); break;
            case 3: ui.set_status_reset_fn(bed_status_reset_bed3); break;
            default: ui.set_status_reset_fn(nullptr); break;
          }
        } 
        else {
          // Sem índice: todas as camas
          ui.set_status_reset_fn(bed_status_reset_all);
        }
      }
    }
  #else //Single Bed Fallback
    void GcodeSuite::M140_M190(const bool isM190) {

      if (DEBUGGING(DRYRUN)) return;

      bool got_temp = false;
      celsius_t temp = 0;

      // Accept 'I' if temperature presets are defined
      #if HAS_PREHEAT
        got_temp = parser.seenval('I');
        if (got_temp) {
          const uint8_t index = parser.value_byte();
          temp = ui.material_preset[_MIN(index, PREHEAT_COUNT - 1)].bed_temp;
        }
      #endif

      // Get the temperature from 'S' or 'R'
      bool no_wait_for_cooling = false;
      if (!got_temp) {
        no_wait_for_cooling = parser.seenval('S');
        got_temp = no_wait_for_cooling || (isM190 && parser.seenval('R'));
        if (got_temp) temp = parser.value_celsius();
      }

      if (!got_temp) return;

      thermalManager.setTargetBed(temp);
      thermalManager.isHeatingBed() ? LCD_MESSAGE(MSG_BED_HEATING) : LCD_MESSAGE(MSG_BED_COOLING);

      // With PRINTJOB_TIMER_AUTOSTART, M190 can start the timer, and M140 can stop it
      TERN_(PRINTJOB_TIMER_AUTOSTART, thermalManager.auto_job_check_timer(isM190, !isM190));

      if (isM190)
        thermalManager.wait_for_bed(no_wait_for_cooling);
      else
        ui.set_status_reset_fn([]{
          const celsius_t c = thermalManager.degTargetBed();
          return c < 30 || thermalManager.degBedNear(c);
        });
    }

  #endif //HAS_MULTI_BEDS 
#endif // HAS_HEATED_BED
