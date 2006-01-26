/* ProfileServ - User Profile Service - NeoStats Addon Module
** Copyright (c) 2006 Justin Hammond, Mark Hetherington, DeadNotBuried
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

#include "neostats.h"    /* Required for bot support */
#include "profileserv.h"

const char *pfs_help_set_exclusions[] = {
	"\2EXCLUSIONS <ON|OFF>\2",
	"Use global exclusion list in addition to local exclusion list",
	NULL
};

const char *pfs_help_set_enable[] = {
	"\2ENABLE <ON|OFF>\2",
	"Enable or Disable ProfileServ usage via private message",
	NULL
};

const char *pfs_help_set_enableprofilechan[] = {
	"\2ENABLEPROFILECHAN <ON|OFF>\2",
	"Enable ProfileServ to join the Profile Channel and accept commands in channel",
	NULL
};

const char *pfs_help_set_profilechanname[] = {
	"\2PROFILECHANNAME <#Channel>\2",
	"Set Channel ProfileServ accepts commands from in channel",
	NULL
};

const char *pfs_help_set_expiredays[] = {
	"\2EXPIREDAYS <#>\2",
	"Sets the number of days after which profile data is",
	"removed, if the nickname has not been seen identified.",
	"Set to 0 to disable Profile expiry.",
	NULL
};

const char *pfs_help_profile[] = {
	"View Profile - Syntax: \2PROFILE <nick>\2",
	"Syntax: \2PROFILE <nick>\2",
	"",
	"Displays infomation stored in ProfileServ for the entered Nickname.",
	NULL
};

const char *pfs_help_realname[] = {
	"Add Real Name to Profile - Syntax: \2REALNAME <real name>\2",
	"Syntax: \2REALNAME <real name>\2",
	"",
	"Add your Real Name to ProfileServ for your current Nick.",
	NULL
};

const char *pfs_help_birthday[] = {
	"Add birthday to Profile - Syntax: \2BIRTHDAY <birthday>\2",
	"Syntax: \2BIRTHDAY <birthday>\2",
	"",
	"Add your birthday to ProfileServ for your current Nick.",
	NULL
};

const char *pfs_help_age[] = {
	"Add Age to Profile - Syntax: \2AGE <age>\2",
	"Syntax: \2AGE <age>\2",
	"",
	"Add your age (in years) to ProfileServ for your current Nick.",
	NULL
};

const char *pfs_help_gender[] = {
	"Add Gender to Profile - Syntax: \2GENDER <gender>\2",
	"Syntax: \2GENDER <gender>\2",
	"",
	"Add your Gender to ProfileServ for your current Nick.",
	NULL
};

const char *pfs_help_maritalstatus[] = {
	"Add Marital Status to Profile - Syntax: \2MARITAL <marital status>\2",
	"Syntax: \2MARITAL <marital status>\2",
	"",
	"Add your Marital Status to ProfileServ for your current Nick.",
	NULL
};

const char *pfs_help_location[] = {
	"Add Location to Profile - Syntax: \2LOCATION <Country / State / Province / City / Suburb>\2",
	"Syntax: \2LOCATION <Country / State / Province / City / Suburb>\2",
	"",
	"Add your Location to ProfileServ for your current Nick.",
	NULL
};

const char *pfs_help_hobbies[] = {
	"Add Hobbies to Profile - Syntax: \2HOBBIES <Hobby List>\2",
	"Syntax: \2HOBBIES <Hobby List>\2",
	"",
	"Add your List of Hobbies to ProfileServ for your current Nick.",
	NULL
};

const char *pfs_help_general[] = {
	"Add General information to Profile - Syntax: \2GENERAL <Line 1-5> <text>\2",
	"Syntax: \2GENERAL <Line 1-5> <text>\2",
	"",
	"Add General information to ProfileServ for your current Nick.",
	"Up to 5 lines of general Information may be entered.",
	"# MUST be a number from 1 to 5, being the line number for the General information.",
	NULL
};

const char *pfs_help_del[] = {
	"Delete a Profile - Syntax: \2DEL <nick>\2",
	"Syntax: \2DEL <nick>\2",
	"",
	"Allows you to delete your profile.",
	"You must enter the nickname to remove the profile.",
	NULL
};
