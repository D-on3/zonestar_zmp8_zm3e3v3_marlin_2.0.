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

#include "../../inc/MarlinConfig.h"

#if ENABLED(SDSUPPORT)

#include "../gcode.h"
#include "../../sd/cardreader.h"
#include "../../module/printcounter.h"
#include "../../lcd/ultralcd.h"

#if ENABLED(PARK_HEAD_ON_PAUSE)
  #include "../../feature/pause.h"
#endif

#if ENABLED(HOST_ACTION_COMMANDS)
  #include "../../feature/host_actions.h"
#endif

#if ENABLED(POWER_LOSS_RECOVERY)
  #include "../../feature/powerloss.h"
#endif

#if HAS_DWIN_LCD
  #include "../../lcd/dwin/dwin_ui/dwin.h"
#endif

#if ENABLED(OPTION_REPEAT_PRINTING)
	#include "../../feature/repeat_printing.h"
#endif


#include "../../MarlinCore.h" // for startOrResumeJob

/**
 * M24: Start or Resume SD Print
 */
void GcodeSuite::M24() {

  #if ENABLED(POWER_LOSS_RECOVERY)
    if (parser.seenval('S')) card.setIndex(parser.value_long());
    if (parser.seenval('T')) print_job_timer.resume(parser.value_long());
  #endif

  #if ENABLED(PARK_HEAD_ON_PAUSE)
    if (did_pause_print) {
      resume_print(); // will call print_job_timer.start()
      return;
    }
  #endif

  if(card.isFileOpen()) {
    card.startFileprint();            								// SD card will now be read for commands
    startOrResumeJob();               								// Start (or resume) the print job timer
    TERN_(POWER_LOSS_RECOVERY, recovery.prepare());		// backup the printing file name
  }

  #if ENABLED(HOST_ACTION_COMMANDS)
    #ifdef ACTION_ON_RESUME
      host_action_resume();
    #endif
    TERN_(HOST_PROMPT_SUPPORT, host_prompt_open(PROMPT_INFO, PSTR("Resuming SD"), DISMISS_STR));
  #endif	
	TERN(HAS_DWIN_LCD, Draw_Printing_Menu(true), ui.reset_status());  
}

/**
 * M25: Pause SD Print
 */
void GcodeSuite::M25() {

  #if ENABLED(PARK_HEAD_ON_PAUSE)

    M125();

  #else

    // Set initial pause flag to prevent more commands from landing in the queue while we try to pause
    #if ENABLED(SDSUPPORT)
      if (IS_SD_PRINTING()) card.pauseSDPrint();
    #endif

    #if ENABLED(POWER_LOSS_RECOVERY)
      recovery.save(true);
    #endif

    print_job_timer.pause();

    #if !HAS_DWIN_LCD
      ui.reset_status();
    #endif

    #if ENABLED(HOST_ACTION_COMMANDS)
      TERN_(HOST_PROMPT_SUPPORT, host_prompt_open(PROMPT_PAUSE_RESUME, PSTR("Pause SD"), PSTR("Resume")));
      #ifdef ACTION_ON_PAUSE
        host_action_pause();
      #endif
    #endif

  #endif
}

#endif // SDSUPPORT
