/**
 * Marlin 3D Printer Firmware
 * Copyright c 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright c 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option any later version.
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
#pragma once

// R25 = 100 kOhm, beta25 = 4092 K, 4.7 kOhm pull-up, bed thermistor
constexpr temp_entry_t temptable_133[] PROGMEM = {
  { OV(  180), -15 },
  { OV(  242), -10 },
  { OV(  323),  -5 },
  { OV(  426),   0 },
  { OV(  555),   5 },
  { OV(  716),  10 },
  { OV(  915),  15 },
  { OV( 1157),  20 },
  { OV( 1448),  25 },
  { OV( 1795),  30 },
  { OV( 2205),  35 },
  { OV( 2682),  40 },
  { OV( 3233),  45 },
  { OV( 3859),  50 },
  { OV( 4564),  55 },
  { OV( 5348),  60 },
  { OV( 6208),  65 },
  { OV( 7140),  70 },
  { OV( 8137),  75 },
  { OV( 9191),  80 },
  { OV(10290),  85 },
  { OV(11422),  90 },
  { OV(12575),  95 },
  { OV(13734), 100 },
  { OV(14888), 105 },
  { OV(16024), 110 },
  { OV(17132), 115 },
  { OV(18203), 120 },
  { OV(19230), 125 },
  { OV(20206), 130 },
  { OV(21129), 135 },
  { OV(21996), 140 },
  { OV(22806), 145 },
  { OV(23560), 150 },
  { OV(24258), 155 },
  { OV(24903), 160 },
  { OV(25497), 165 },
  { OV(26043), 170 },
  { OV(26544), 175 },
  { OV(27002), 180 },
  { OV(27421), 185 },
  { OV(27804), 190 },
  { OV(28154), 195 },
  { OV(28474), 200 },
  { OV(28765), 205 },
  { OV(29032), 210 },
  { OV(29275), 215 },
  { OV(29497), 220 },
  { OV(29700), 225 },
  { OV(29885), 230 },
  { OV(30055), 235 },
  { OV(30211), 240 },
  { OV(30353), 245 },
  { OV(30484), 250 },
  { OV(30604), 255 },
  { OV(30714), 260 },
  { OV(30816), 265 },
  { OV(30909), 270 },
  { OV(30995), 275 },
  { OV(31074), 280 },
  { OV(31147), 285 },
  { OV(31215), 290 },
  { OV(31278), 295 },
  { OV(31335), 300 },
};