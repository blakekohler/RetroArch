/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2015 - Daniel De Matteis
 *  Copyright (C) 2012-2015 - Michael Lelli
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <compat/strl.h>

#include "menu_generic.h"

#include "../menu.h"
#include "../menu_animation.h"
#include "../menu_display.h"
#include "../menu_displaylist.h"
#include "../menu_hash.h"
#include "../menu_list.h"

#include "../../configuration.h"
#include "../../performance.h"
#include "../../input/input_autodetect.h"
#include "../../input/input_common.h"

#include "../../runloop.h"

static int action_iterate_help(char *s, size_t len, const char *label)
{
   menu_handle_t *menu       = menu_driver_get_ptr();
   settings_t *settings      = config_get_ptr();

   switch (menu->help_screen_type)
   {
      case MENU_HELP_WELCOME:
         {
            static int64_t timeout_end;
            int64_t timeout;
            static bool timer_begin = false;
            static bool timer_end   = false;
            int64_t current         = retro_get_time_usec();

            if (!timer_begin)
            {
               timeout_end = retro_get_time_usec() +
                  3 /* seconds */ * 1000000;
               timer_begin = true;
               timer_end   = false;
            }

            timeout = (timeout_end - current) / 1000000;

            menu_hash_get_help(MENU_LABEL_WELCOME_TO_RETROARCH,
                  s, len);

            if (!timer_end && timeout <= 0)
            {
               timer_end   = true;
               timer_begin = false;
               timeout_end = 0;
               menu->help_screen_type = MENU_HELP_NONE;
               return 1;
            }
         }
         break;
      case MENU_HELP_CONTROLS:
         {
            unsigned i;
            char s2[PATH_MAX_LENGTH];
            const unsigned binds[] = {
               RETRO_DEVICE_ID_JOYPAD_UP,
               RETRO_DEVICE_ID_JOYPAD_DOWN,
               RETRO_DEVICE_ID_JOYPAD_A,
               RETRO_DEVICE_ID_JOYPAD_B,
               RETRO_DEVICE_ID_JOYPAD_SELECT,
               RETRO_DEVICE_ID_JOYPAD_START,
               RARCH_MENU_TOGGLE,
               RARCH_QUIT_KEY,
               RETRO_DEVICE_ID_JOYPAD_X,
               RETRO_DEVICE_ID_JOYPAD_Y,
            };
            char desc[ARRAY_SIZE(binds)][64] = {{0}};

            for (i = 0; i < ARRAY_SIZE(binds); i++)
            {
               const struct retro_keybind *keybind = (const struct retro_keybind*)
                  &settings->input.binds[0][binds[i]];
               const struct retro_keybind *auto_bind = (const struct retro_keybind*)
                  input_get_auto_bind(0, binds[i]);

               input_get_bind_string(desc[i], keybind, auto_bind, sizeof(desc[i]));
            }

            menu_hash_get_help(MENU_LABEL_VALUE_MENU_CONTROLS_PROLOG,
                  s2, sizeof(s2));

            snprintf(s, len,
                  "%s"
                  "[%s]: "
                  "%-20s\n"
                  "[%s]: "
                  "%-20s\n"
                  "[%s]: "
                  "%-20s\n"
                  "[%s]: "
                  "%-20s\n"
                  "[%s]: "
                  "%-20s\n"
                  "[%s]: "
                  "%-20s\n"
                  "[%s]: "
                  "%-20s\n"
                  "[%s]: "
                  "%-20s\n"
                  "[%s]: "
                  "%-20s\n",
                  s2,
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_SCROLL_UP),    desc[0],
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_SCROLL_DOWN),  desc[1],
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_CONFIRM),      desc[2],
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_BACK),         desc[3],
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_INFO),         desc[4],
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_START),        desc[5],
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_TOGGLE_MENU),  desc[6],
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_QUIT),         desc[7],
                  menu_hash_to_str(MENU_LABEL_VALUE_BASIC_MENU_CONTROLS_TOGGLE_KEYBOARD), desc[8]
                  );
         }
         break;
      case MENU_HELP_WHAT_IS_A_CORE:
         menu_hash_get_help(MENU_LABEL_VALUE_WHAT_IS_A_CORE_DESC,
               s, len);
         break;
      case MENU_HELP_LOADING_CONTENT:
         menu_hash_get_help(MENU_LABEL_LOAD_CONTENT,
               s, len);
         break;
      case MENU_HELP_CHANGE_VIRTUAL_GAMEPAD:
         menu_hash_get_help(MENU_LABEL_VALUE_HELP_CHANGE_VIRTUAL_GAMEPAD_DESC,
               s, len);
         break;
      case MENU_HELP_AUDIO_VIDEO_TROUBLESHOOTING:
         menu_hash_get_help(MENU_LABEL_VALUE_HELP_AUDIO_VIDEO_TROUBLESHOOTING_DESC,
               s, len);
         break;
      case MENU_HELP_SCANNING_CONTENT:
         menu_hash_get_help(MENU_LABEL_VALUE_HELP_SCANNING_CONTENT_DESC,
               s, len);
         break;
      case MENU_HELP_EXTRACT:
         menu_hash_get_help(MENU_LABEL_VALUE_EXTRACTING_PLEASE_WAIT,
               s, len);
         break;
      case MENU_HELP_NONE:
      default:
         break;
   }


   return 0;
}

static enum action_iterate_type action_iterate_type(uint32_t hash)
{
   switch (hash)
   {
      case MENU_LABEL_HELP:
      case MENU_LABEL_HELP_CONTROLS:
      case MENU_LABEL_HELP_WHAT_IS_A_CORE:
      case MENU_LABEL_HELP_LOADING_CONTENT:
      case MENU_LABEL_HELP_CHANGE_VIRTUAL_GAMEPAD:
      case MENU_LABEL_HELP_AUDIO_VIDEO_TROUBLESHOOTING:
      case MENU_LABEL_HELP_SCANNING_CONTENT:
         return ITERATE_TYPE_HELP;
      case MENU_LABEL_INFO_SCREEN:
         return ITERATE_TYPE_INFO;
      case MENU_LABEL_CUSTOM_BIND:
      case MENU_LABEL_CUSTOM_BIND_ALL:
      case MENU_LABEL_CUSTOM_BIND_DEFAULTS:
         return ITERATE_TYPE_BIND;
   }

   return ITERATE_TYPE_DEFAULT;
}

/**
 * menu_iterate:
 * @input                    : input sample for this frame
 * @old_input                : input sample of the previous frame
 * @trigger_input            : difference' input sample - difference
 *                             between 'input' and 'old_input'
 *
 * Runs RetroArch menu for one frame.
 *
 * Returns: 0 on success, -1 if we need to quit out of the loop. 
 **/
int generic_menu_iterate(enum menu_action action)
{
   size_t selection;
   menu_entry_t entry;
   enum action_iterate_type iterate_type;
   const char *label         = NULL;
   int ret                   = 0;
   uint32_t label_hash       = 0;
   uint32_t hash             = 0;
   menu_handle_t *menu       = menu_driver_get_ptr();
   menu_list_t *menu_list    = menu_list_get_ptr();

   menu_list_get_last_stack(menu_list, NULL, &label, NULL, NULL);

   if (!menu || !menu_list)
      return 0;
   if (!menu_navigation_ctl(MENU_NAVIGATION_CTL_GET_SELECTION, &selection))
      return 0;

   menu->menu_state.msg[0]   = '\0';
   hash                      = menu_hash_calculate(label);
   iterate_type              = action_iterate_type(hash);

   if (action != MENU_ACTION_NOOP || menu_entries_needs_refresh() || menu_display_ctl(MENU_DISPLAY_CTL_UPDATE_PENDING, NULL))
   {
      BIT64_SET(menu->state, MENU_STATE_RENDER_FRAMEBUFFER);
   }

   switch (iterate_type)
   {
      case ITERATE_TYPE_HELP:
         ret = action_iterate_help(menu->menu_state.msg, sizeof(menu->menu_state.msg), label);
         BIT64_SET(menu->state, MENU_STATE_RENDER_MESSAGEBOX);
         BIT64_SET(menu->state, MENU_STATE_POST_ITERATE);
         if (ret == 1 || action == MENU_ACTION_OK)
            BIT64_SET(menu->state, MENU_STATE_POP_STACK);
         break;
      case ITERATE_TYPE_BIND:
         if (menu_input_key_bind_iterate(menu->menu_state.msg, sizeof(menu->menu_state.msg)))
         {
            menu_list_pop_stack(menu_list, &selection);
            menu_navigation_ctl(MENU_NAVIGATION_CTL_SET_SELECTION, &selection);
         }
         else
            BIT64_SET(menu->state, MENU_STATE_RENDER_MESSAGEBOX);
         break;
      case ITERATE_TYPE_INFO:
         {
            menu_file_list_cbs_t *cbs = menu_list_get_actiondata_at_offset(menu_list->selection_buf, selection);
            rarch_setting_t *setting  = cbs->setting;

            if (setting)
            {
               char needle[PATH_MAX_LENGTH];
               strlcpy(needle, menu_setting_get_name(setting), sizeof(needle));
               label_hash       = menu_hash_calculate(needle);
            }

            ret = menu_hash_get_help(label_hash, menu->menu_state.msg, sizeof(menu->menu_state.msg));
         }
         BIT64_SET(menu->state, MENU_STATE_RENDER_MESSAGEBOX);
         BIT64_SET(menu->state, MENU_STATE_POST_ITERATE);
         if (action == MENU_ACTION_OK)
            BIT64_SET(menu->state, MENU_STATE_POP_STACK);
         break;
      case ITERATE_TYPE_DEFAULT:
         /* FIXME: Crappy hack, needed for mouse controls to not be completely broken
          * in case we press back.
          *
          * We need to fix this entire mess, mouse controls should not rely on a 
          * hack like this in order to work. */
         selection = max(min(selection, menu_list_get_size(menu_list)-1), 0);

         menu_entry_get(&entry,    selection, NULL, false);
         ret = menu_entry_action(&entry, selection, (enum menu_action)action);

         if (ret)
            goto end;

         BIT64_SET(menu->state, MENU_STATE_POST_ITERATE);

         /* Have to defer it so we let settings refresh. */
         if (menu->push_help_screen)
         {
            menu_displaylist_info_t info = {0};

            info.list = menu_list->menu_stack;
            strlcpy(info.label,
                  menu_hash_to_str(MENU_LABEL_HELP),
                  sizeof(info.label));

            menu_displaylist_push_list(&info, DISPLAYLIST_HELP);
         }
         break;
   }

   BIT64_SET(menu->state, MENU_STATE_BLIT);

   if (BIT64_GET(menu->state, MENU_STATE_POP_STACK))
   {
      size_t new_selection_ptr = selection;
      menu_list_pop_stack(menu_list, &new_selection_ptr);
      menu_navigation_ctl(MENU_NAVIGATION_CTL_SET_SELECTION, &selection);
   }
   
   if (BIT64_GET(menu->state, MENU_STATE_POST_ITERATE))
      menu_input_post_iterate(&ret, action);

end:
   if (ret)
      return -1;
   return 0;
}

int menu_iterate_render(void)
{
   bool is_idle;
   const menu_ctx_driver_t *driver = menu_ctx_driver_get_ptr();
   menu_handle_t *menu       = menu_driver_get_ptr();

   if (!menu)
      return -1;

   if (BIT64_GET(menu->state, MENU_STATE_RENDER_FRAMEBUFFER) != BIT64_GET(menu->state, MENU_STATE_RENDER_MESSAGEBOX))
      BIT64_SET(menu->state, MENU_STATE_RENDER_FRAMEBUFFER);

   if (BIT64_GET(menu->state, MENU_STATE_RENDER_FRAMEBUFFER))
      menu_display_ctl(MENU_DISPLAY_CTL_SET_FRAMEBUFFER_DIRTY_FLAG, NULL);

   if (BIT64_GET(menu->state, MENU_STATE_RENDER_MESSAGEBOX) && menu->menu_state.msg[0] != '\0')
   {
      if (driver->render_messagebox)
         driver->render_messagebox(menu->menu_state.msg);

      if (ui_companion_is_on_foreground())
      {
         const ui_companion_driver_t *ui = ui_companion_get_ptr();
         if (ui->render_messagebox)
            ui->render_messagebox(menu->menu_state.msg);
      }
   }
      
   if (BIT64_GET(menu->state, MENU_STATE_BLIT))
   {
      menu_animation_ctl(MENU_ANIMATION_CTL_UPDATE_TIME, NULL);
      if (driver->render)
         driver->render();
   }

   rarch_main_ctl(RARCH_MAIN_CTL_IS_IDLE, &is_idle);

   if (menu_driver_alive() && !is_idle)
      menu_display_ctl(MENU_DISPLAY_CTL_LIBRETRO, NULL);

   menu_driver_set_texture();

   menu->state               = 0;

   return 0;
}

bool generic_menu_init_list(void *data)
{
   menu_displaylist_info_t info = {0};
   menu_handle_t   *menu  = (menu_handle_t*)data;
   menu_list_t *menu_list = menu_list_get_ptr();

   if (!menu || !menu_list)
      return false;

   info.list  = menu_list->selection_buf;
   info.type  = MENU_SETTINGS;
   info.flags = SL_FLAG_MAIN_MENU | SL_FLAG_MAIN_MENU_SETTINGS;
   strlcpy(info.label, menu_hash_to_str(MENU_VALUE_MAIN_MENU), sizeof(info.label));

   menu_list_push(menu_list->menu_stack,
         info.path, info.label, info.type, info.flags, 0);
   menu_displaylist_push_list(&info, DISPLAYLIST_MAIN_MENU);

   return true;
}
