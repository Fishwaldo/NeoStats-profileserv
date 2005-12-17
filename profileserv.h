/* ProfileServ - User Profile Service - NeoStats Addon Module
** Copyright (c) 2005 Justin Hammond, Mark Hetherington, DeadNotBuried
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** ProfileServ CVS Identification
** $Id$
*/

#include MODULECONFIG

/* Defines */
#define PFS_TYPESIZE		3			/* Type Field Size */
#define PFS_TEXTSIZE		300			/* Text Field Size */
#define PFS_TYPENICKSIZE	PFS_TYPESIZE+MAXNICK	/* Type and Nick Key Field Size */

/* Variables And Structs */
Bot *pfs_bot;

struct ProfileServ {
	int exclusions;
	int enable;
	int enableprofilechan;
	char profilechan[MAXCHANLEN];
	int expiredays;
} ProfileServ;

typedef struct ProfileUser {
	char nick[MAXNICK];
	int lastseen;
} ProfileUser;

typedef struct ProfileData {
	char typenick[PFS_TYPENICKSIZE];
	char typedata[PFS_TEXTSIZE];
} ProfileData;

/* ProfileServ Module Help - ProfileServ_help.c */
extern const char *pfs_help_set_exclusions[];
extern const char *pfs_help_set_enable[];
extern const char *pfs_help_set_enableprofilechan[];
extern const char *pfs_help_set_profilechanname[];
extern const char *pfs_help_set_expiredays[];
extern const char *pfs_help_profile[];
extern const char *pfs_help_realname[];
extern const char *pfs_help_birthday[];
extern const char *pfs_help_age[];
extern const char *pfs_help_gender[];
extern const char *pfs_help_maritalstatus[];
extern const char *pfs_help_location[];
extern const char *pfs_help_hobbies[];
extern const char *pfs_help_general[];
extern const char *pfs_help_del[];

/* profileserv.c */
int pfs_set_exclusions( const CmdParams *cmdparams, SET_REASON reason );
int pfs_set_enablechan (const CmdParams *cmdparams, SET_REASON reason);
int pfs_set_profilechan (const CmdParams *cmdparams, SET_REASON reason);
void profile_report( const CmdParams *cmdparams, const char *fmt, ... );
int pfs_cmd_profile (const CmdParams *cmdparams);
int pfs_cmd_realname (const CmdParams *cmdparams);
int pfs_cmd_birthday (const CmdParams *cmdparams);
int pfs_cmd_age (const CmdParams *cmdparams);
int pfs_cmd_gender (const CmdParams *cmdparams);
int pfs_cmd_maritalstatus (const CmdParams *cmdparams);
int pfs_cmd_location (const CmdParams *cmdparams);
int pfs_cmd_hobbies (const CmdParams *cmdparams);
int pfs_cmd_general (const CmdParams *cmdparams);
int pfs_cmd_del (const CmdParams *cmdparams);
void pfs_delete_profile ( const CmdParams *cmdparams, ProfileUser *pu );
int pfs_expire_profiles(void*);
int expireoldprofiles(void *data, int size);
int pfs_UmodeUser (const CmdParams *cmdparams);
