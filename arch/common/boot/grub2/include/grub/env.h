/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2005,2006,2007  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_ENV_HEADER
#define GRUB_ENV_HEADER	1

#include <grub/symbol.h>
#include <grub/err.h>
#include <grub/types.h>

struct grub_env_var;

typedef char *(*grub_env_read_hook_t) (struct grub_env_var *var,
				       const char *val);
typedef char *(*grub_env_write_hook_t) (struct grub_env_var *var,
					const char *val);

enum grub_env_var_type
  {
    /* The default variable type which is local in current context.  */
    GRUB_ENV_VAR_LOCAL,

    /* The exported type, which is passed to new contexts.  */
    GRUB_ENV_VAR_GLOBAL,

    /* The data slot type, which is used to store arbitrary data.  */
    GRUB_ENV_VAR_DATA
  };

struct grub_env_var
{
  char *name;
  char *value;
  grub_env_read_hook_t read_hook;
  grub_env_write_hook_t write_hook;
  struct grub_env_var *next;
  struct grub_env_var **prevp;
  enum grub_env_var_type type;
};

grub_err_t EXPORT_FUNC(grub_env_set) (const char *name, const char *val);
char *EXPORT_FUNC(grub_env_get) (const char *name);
void EXPORT_FUNC(grub_env_unset) (const char *name);
void EXPORT_FUNC(grub_env_iterate) (int (*func) (struct grub_env_var *var));
grub_err_t EXPORT_FUNC(grub_register_variable_hook) (const char *name,
						     grub_env_read_hook_t read_hook,
						     grub_env_write_hook_t write_hook);
grub_err_t EXPORT_FUNC(grub_env_context_open) (void);
grub_err_t EXPORT_FUNC(grub_env_context_close) (void);
grub_err_t EXPORT_FUNC(grub_env_export) (const char *name);

grub_err_t EXPORT_FUNC(grub_env_set_data_slot) (const char *name,
						const void *ptr);
void *EXPORT_FUNC(grub_env_get_data_slot) (const char *name);
void EXPORT_FUNC(grub_env_unset_data_slot) (const char *name);

#endif /* ! GRUB_ENV_HEADER */
