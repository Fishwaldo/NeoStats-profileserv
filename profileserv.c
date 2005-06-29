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

#include "neostats.h"    /* Required for bot support */
#include "profileserv.h"

/* data */
char nicklower[MAXNICK];

/** Copyright info */
const char *pfs_copyright[] = {
	"Copyright (c) 2005, NeoStats",
	"http://www.neostats.net/",
	NULL};

const char *pfs_about[] = {
	"\2Profile Service\2",
	"",
	"Provides User Profile Services.",
	"Allows saving of data provided by the user of the",
	"registered nickname, and viewing by other users.",
	NULL
};

/*
 * Commands and Settings
*/
static bot_cmd pfs_commands[]=
{
	{"VIEW",		pfs_cmd_view,		1,	0,	pfs_help_view},
	{"REALNAME",		pfs_cmd_realname,	1,	0,	pfs_help_realname},
	{"BIRTHDAY",		pfs_cmd_birthday,	3,	0,	pfs_help_birthday},
	{"AGE",			pfs_cmd_age,		1,	0,	pfs_help_age},
	{"GENDER",		pfs_cmd_gender,		1,	0,	pfs_help_gender},
	{"MARITALSTATUS",	pfs_cmd_maritalstatus,	1,	0,	pfs_help_maritalstatus},
	{"LOCATION",		pfs_cmd_location,	1,	0,	pfs_help_location},
	{"HOBBIES",		pfs_cmd_hobbies,	1,	0,	pfs_help_hobbies},
	{"GENERAL",		pfs_cmd_general,	2,	0,	pfs_help_general},
	{"DEL",			pfs_cmd_del,		1,	0,	pfs_help_del},
	{NULL,			NULL,			0, 	0,	NULL}
};

static bot_setting pfs_settings[]=
{
	{"EXCLUSIONS",		&ProfileServ.exclusions,	SET_TYPE_BOOLEAN,	0,	0,		NS_ULEVEL_ADMIN,	NULL,	pfs_help_set_exclusions,	pfs_set_exclusions,	(void *)1 },
	{"ENABLE",		&ProfileServ.enable,		SET_TYPE_BOOLEAN,	0,	0,		NS_ULEVEL_ADMIN,	NULL,	pfs_help_set_enable,		NULL,			(void *)0 },
	{"ENABLEPROFILECHAN",	&ProfileServ.enableprofilechan,	SET_TYPE_BOOLEAN,	0,	0,		NS_ULEVEL_ADMIN,	NULL,	pfs_help_set_enableprofilechan,	pfs_set_enablechan,	(void *)0 },
	{"PROFILECHANNAME",	&ProfileServ.profilechan,	SET_TYPE_CHANNEL,	0,	MAXCHANLEN,	NS_ULEVEL_ADMIN,	NULL,	pfs_help_set_profilechan,	pfs_set_profilechan,	(void *)"#Profile" },
	{"EXPIREDAYS",		&ProfileServ.expiredays,	SET_TYPE_INT,		0,	100000,		NS_ULEVEL_ADMIN,	NULL,	pfs_help_set_expiredays,	NULL,			(void *)30 },
	{NULL,			NULL,				0,			0,	0,		0,			NULL,	NULL,				NULL, 			NULL },
};

/*
 * Module Info definition 
*/
ModuleInfo module_info = {
	"ProfileServ",
	"Profile Service Module For NeoStats",
	pfs_copyright,
	pfs_about,
	NEOSTATS_VERSION,
	"3.0",
	__DATE__,
	__TIME__,
	MODULE_FLAG_LOCAL_EXCLUDES,
	0,
};

/*
 * Module event list
*/
ModuleEvent module_events[] = {
	{EVENT_UMODE,	pfs_UmodeUser},
	{EVENT_NULL,	NULL}
};

/** BotInfo */
static BotInfo pfs_botinfo = 
{
	"ProfileServ",
	"ProfileServ1",
	"Profiles",
	BOT_COMMON_HOST,
	"Profile Service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_PERSIST,
	pfs_commands,
	pfs_settings,
};

/*
 * Online event processing
*/
int ModSynch (void)
{
	/* Introduce a bot onto the network */
	pfs_bot = AddBot (&pfs_botinfo);	
	if (!pfs_bot)
		return NS_FAILURE;
	if (ProfileServ.enableprofilechan) {
		irc_join (pfs_bot, ProfileServ.profilechan, "+o");
		irc_chanalert (pfs_bot, "Profile Channel Now Available in %s", ProfileServ.profilechan);
	} else {
		irc_chanalert (pfs_bot, "Profile Channel Not Enabled");
	}
	return NS_SUCCESS;
};

/*
 * Init module
*/
int ModInit( void )
{
	ModuleConfig (pfs_settings);
	AddTimer (TIMER_TYPE_DAILY, pfs_expire_profiles, "pfs_expire_profiles", 0);
	return NS_SUCCESS;
}

/*
 * Exit module
*/
int ModFini( void )
{
	DelTimer ("pfs_expire_profiles");
	return NS_SUCCESS;
}

/*
 * Enable/Disable Global Exclusions
*/
int pfs_set_exclusions( CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
		SetAllEventFlags( EVENT_FLAG_USE_EXCLUDE, ProfileServ.exclusions );
	return NS_SUCCESS;
}

/*
 * Profile Channel Enable/Disable
*/
int pfs_set_enablechan (CmdParams *cmdparams, SET_REASON reason) 
{
	if (!ProfileServ.profilechan)
		return NS_SUCCESS;
	if (reason == SET_CHANGE) 
	{
		if (ProfileServ.enableprofilechan) 
		{
			irc_join (pfs_bot, ProfileServ.profilechan, "+o");
			irc_chanalert (pfs_bot, "Profile functions now available in %s", ProfileServ.profilechan);
			return NS_SUCCESS;
		} else {
			irc_part (pfs_bot, ProfileServ.profilechan, NULL);
			irc_chanalert (pfs_bot, "Profile functions are no longer available in %s", ProfileServ.profilechan);
			return NS_SUCCESS;
		}
	}
	return NS_SUCCESS;
}

/*
 * Profile Channel Setting
*/
int pfs_set_profilechan (CmdParams *cmdparams, SET_REASON reason) 
{
	if (!ProfileServ.enableprofilechan)
		return NS_SUCCESS;
	if (reason == SET_VALIDATE) 
	{
		irc_prefmsg (pfs_bot, cmdparams->source, "Profile Channel changing from %s to %s", ProfileServ.profilechan, cmdparams->av[1]);
		irc_chanalert (pfs_bot, "Profile Channel Changing to %s , Parting %s (%s)", cmdparams->av[1], ProfileServ.profilechan, cmdparams->source->name);
		irc_chanprivmsg (pfs_bot, ProfileServ.profilechan, "\0039%s has changed Channels, Profile functions will now be available in %s", cmdparams->source->name, cmdparams->av[1]);
		irc_part (pfs_bot, ProfileServ.profilechan, NULL);
		return NS_SUCCESS;
	}
	if (reason == SET_CHANGE) 
	{
		irc_join (pfs_bot, ProfileServ.profilechan, "+o");
		irc_chanalert (pfs_bot, "Profile functions now available in %s", ProfileServ.profilechan);
		return NS_SUCCESS;
	}
	return NS_SUCCESS;
}

/** profile_report
 *
 *  handles channel/user message selection
 */
static char profile_report_buf[BUFSIZE];

void profile_report( CmdParams *cmdparams, const char *fmt, ... )
{
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( profile_report_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	if( cmdparams->channel == NULL )
		irc_prefmsg (pfs_bot, cmdparams->source, profile_report_buf );
	else
		irc_chanprivmsg (pfs_bot, cmdparams->channel->name, profile_report_buf );
}


/*
 * View Nicknames Profile
*/
int pfs_cmd_view (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if( ValidateNick( cmdparams->av[0] ) == NS_FAILURE ) 
	{
		profile_report( cmdparams, "Invalid Nickname Specified" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( nicklower, cmdparams->av[0], MAXNICK );
	if( DBAFetch( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) ) == NS_FAILURE )
	{
		profile_report( cmdparams, "No Profile exists for specified nickname" );
	}
	else
	{
		profile_report( cmdparams, "Profile For : %s", pu->nick );
		strlcpy( pd->typenick, "RNT" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, "Real Name : %s", pd->typedata );
		strlcpy( pd->typenick, "BDT" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, "Birthday : %s", pd->typedata );
		strlcpy( pd->typenick, "AGE" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, "Age : %s", pd->typedata );
		strlcpy( pd->typenick, "SEX" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, "Gender : %s", pd->typedata );
		strlcpy( pd->typenick, "MST" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, "Marital Status : %s", pd->typedata );
		strlcpy( pd->typenick, "LOC" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, "Location : %s", pd->typedata );
		strlcpy( pd->typenick, "HOB" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, "Hobbies : %s", pd->typedata );
		strlcpy( pd->typenick, "GT1" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, pd->typedata );
		strlcpy( pd->typenick, "GT2" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, pd->typedata );
		strlcpy( pd->typenick, "GT3" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, pd->typedata );
		strlcpy( pd->typenick, "GT4" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, pd->typedata );
		strlcpy( pd->typenick, "GT5" , PFS_TYPENICKSIZE );
		strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
		if( DBAFetch( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) ) != NS_FAILURE )
			profile_report( cmdparams, pd->typedata );
		profile_report( cmdparams, "End of Profile For : %s", pu->nick );
	}
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Set Real Name
*/
int pfs_cmd_realname (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	char *buf;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if ( !(cmdparams->source->user->Umode & UMODE_REGNICK) )
	{
		profile_report( cmdparams, "You must be Identified for your nickname to add data to the profile" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
	strlcpy( nicklower, cmdparams->source->name , MAXNICK );
	pu->lastseen = me.now;
	DBAStore( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) );
	buf= joinbuf(cmdparams->av, cmdparams->ac, 0);
	strlcpy( pd->typenick, "RNT" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	strlcpy( pd->typedata, buf, PFS_TEXTSIZE );
	DBAStore( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) );
	profile_report( cmdparams, "Real Name changed to : %s", pd->typedata );
	ns_free( buf );
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Set Birthday
*/
int pfs_cmd_birthday (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	int d, m, y;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if ( !(cmdparams->source->user->Umode & UMODE_REGNICK) )
	{
		profile_report( cmdparams, "You must be Identified for your nickname to add data to the profile" );
		return NS_SUCCESS;
	}
	d = atoi(cmdparams->av[0]);
	m = atoi(cmdparams->av[1]);
	y = atoi(cmdparams->av[2]);
	if ( d < 1 || d > 31 || m < 1 || m > 12 )
	{
		profile_report( cmdparams, "Invalid Date Specifed (must be in the format 'dd mm yy')" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
	strlcpy( nicklower, cmdparams->source->name , MAXNICK );
	pu->lastseen = me.now;
	DBAStore( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) );
	strlcpy( pd->typenick, "BDT" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	ircsnprintf( pd->typedata, PFS_TEXTSIZE, "%d %s %d", d , (m == 1) ? "Jan" : (m == 2) ? "Feb" : (m == 3) ? "Mar" : (m == 4) ? "Apr" : (m == 5) ? "May" : (m == 6) ? "Jun" : (m == 7) ? "Jul" : (m == 8) ? "Aug" : (m == 9) ? "Sep" : (m == 10) ? "Oct" : (m == 11) ? "Nov" : "Dec", y);
	DBAStore( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) );
	profile_report( cmdparams, "Birthday changed to : %s", pd->typedata );
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Set Age
*/
int pfs_cmd_age (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	int userage;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if ( !(cmdparams->source->user->Umode & UMODE_REGNICK) )
	{
		profile_report( cmdparams, "You must be Identified for your nickname to add data to the profile" );
		return NS_SUCCESS;
	}
	userage = atoi(cmdparams->av[0]);
	if ( userage < 5 || userage > 125 )
	{
		profile_report( cmdparams, "Invalid Age Specifed" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
	strlcpy( nicklower, cmdparams->source->name , MAXNICK );
	pu->lastseen = me.now;
	DBAStore( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) );
	strlcpy( pd->typenick, "AGE" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	ircsnprintf( pd->typedata, PFS_TEXTSIZE, "%d", userage);
	DBAStore( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) );
	profile_report( cmdparams, "Age changed to : %s", pd->typedata );
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Set Gender
*/
int pfs_cmd_gender (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if ( !(cmdparams->source->user->Umode & UMODE_REGNICK) )
	{
		profile_report( cmdparams, "You must be Identified for your nickname to add data to the profile" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
	strlcpy( nicklower, cmdparams->source->name , MAXNICK );
	pu->lastseen = me.now;
	DBAStore( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) );
	strlcpy( pd->typenick, "SEX" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	switch (cmdparams->av[0][0])
	{
		case 'm':
		case 'M':
			strlcpy( pd->typedata, "Male", PFS_TEXTSIZE );
			break;
		case 'f':
		case 'F':
			strlcpy( pd->typedata, "Female", PFS_TEXTSIZE );
			break;
		case 'a':
		case 'A':
			strlcpy( pd->typedata, "Alien", PFS_TEXTSIZE );
			break;
		case 'o':
		case 'O':
			strlcpy( pd->typedata, "Other", PFS_TEXTSIZE );
			break;
		case 'u':
		case 'U':
			strlcpy( pd->typedata, "Unknown", PFS_TEXTSIZE );
			break;
		case 'i':
		case 'I':
			strlcpy( pd->typedata, "I'm checking on that myself right now ;p", PFS_TEXTSIZE );
			break;
		default:
			pd->typedata[0] = '\0';
			break;
	}
	if( strlen( pd->typedata ) < 1 ) {
		profile_report( cmdparams, "Invalid Gender Specified");
	} else {
		DBAStore( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) );
		profile_report( cmdparams, "Gender changed to : %s", pd->typedata );
	}
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Set Marital Status
*/
int pfs_cmd_maritalstatus (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if ( !(cmdparams->source->user->Umode & UMODE_REGNICK) )
	{
		profile_report( cmdparams, "You must be Identified for your nickname to add data to the profile" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
	strlcpy( nicklower, cmdparams->source->name , MAXNICK );
	pu->lastseen = me.now;
	DBAStore( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) );
	strlcpy( pd->typenick, "MST" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	switch (cmdparams->av[0][0])
	{
		case 's':
		case 'S':
			strlcpy( pd->typedata, "Single", PFS_TEXTSIZE );
			break;
		case 'm':
		case 'M':
			strlcpy( pd->typedata, "Married", PFS_TEXTSIZE );
			break;
		case 'd':
		case 'D':
			strlcpy( pd->typedata, "Divorced", PFS_TEXTSIZE );
			break;
		case 'a':
		case 'A':
			strlcpy( pd->typedata, "Available", PFS_TEXTSIZE );
			break;
		case 'u':
		case 'U':
			strlcpy( pd->typedata, "Unavailable", PFS_TEXTSIZE );
			break;
		case 'w':
		case 'W':
			strlcpy( pd->typedata, "Getting rid of the Witch", PFS_TEXTSIZE );
			break;
		case 'b':
		case 'B':
			strlcpy( pd->typedata, "Getting rid of the Hubastard", PFS_TEXTSIZE );
			break;
		case 't':
		case 'T':
			strlcpy( pd->typedata, "Too Bloody Picky", PFS_TEXTSIZE );
			break;
		case 'p':
		case 'P':
			strlcpy( pd->typedata, "Waiting (because there is no Price Charming/Sleeping Beauty)", PFS_TEXTSIZE );
			break;
		default:
			pd->typedata[0] = '\0';
			break;
	}
	if( strlen( pd->typedata ) < 1 ) {
		profile_report( cmdparams, "Invalid Marital Status Specified");
	} else {
		DBAStore( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) );
		profile_report( cmdparams, "Marital Status changed to : %s", pd->typedata );
	}
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Set Location
*/
int pfs_cmd_location (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	char *buf;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if ( !(cmdparams->source->user->Umode & UMODE_REGNICK) )
	{
		profile_report( cmdparams, "You must be Identified for your nickname to add data to the profile" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
	strlcpy( nicklower, cmdparams->source->name , MAXNICK );
	pu->lastseen = me.now;
	DBAStore( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) );
	buf= joinbuf(cmdparams->av, cmdparams->ac, 0);
	strlcpy( pd->typenick, "LOC" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	strlcpy( pd->typedata, buf, PFS_TEXTSIZE );
	DBAStore( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) );
	profile_report( cmdparams, "Location changed to : %s", pd->typedata );
	ns_free( buf );
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Set Hobbies
*/
int pfs_cmd_hobbies (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	char *buf;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if ( !(cmdparams->source->user->Umode & UMODE_REGNICK) )
	{
		profile_report( cmdparams, "You must be Identified for your nickname to add data to the profile" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
	strlcpy( nicklower, cmdparams->source->name , MAXNICK );
	pu->lastseen = me.now;
	DBAStore( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) );
	buf= joinbuf(cmdparams->av, cmdparams->ac, 0);
	strlcpy( pd->typenick, "HOB" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	strlcpy( pd->typedata, buf, PFS_TEXTSIZE );
	DBAStore( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) );
	profile_report( cmdparams, "Hobbies changed to : %s", pd->typedata );
	ns_free( buf );
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Set General Information Lines
*/
int pfs_cmd_general (CmdParams *cmdparams)
{
	ProfileUser *pu;
	ProfileData *pd;
	char *buf;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if ( !(cmdparams->source->user->Umode & UMODE_REGNICK) )
	{
		profile_report( cmdparams, "You must be Identified for your nickname to add data to the profile" );
		return NS_SUCCESS;
	}
	if ( atoi(cmdparams->av[0]) < 1 || atoi(cmdparams->av[0]) > 5 || strlen(cmdparams->av[0]) > 1 )
	{
		profile_report( cmdparams, "Invalid General Line Number, Must be numbered 1 to 5" );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
	strlcpy( nicklower, cmdparams->source->name , MAXNICK );
	pu->lastseen = me.now;
	DBAStore( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) );
	buf= joinbuf(cmdparams->av, cmdparams->ac, 1);
	strlcpy( pd->typenick, "GT" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, cmdparams->av[0] , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	strlcpy( pd->typedata, buf, PFS_TEXTSIZE );
	DBAStore( "UserData", pd->typenick, ( void * )pd, sizeof( ProfileData ) );
	profile_report( cmdparams, "General Information Line %d changed to : %s", cmdparams->av[0], pd->typedata );
	ns_free( buf );
	ns_free( pu );
	ns_free( pd );
	return NS_SUCCESS;
}

/*
 * Delete Nicknames Profile Command
*/
int pfs_cmd_del (CmdParams *cmdparams)
{
	ProfileUser *pu;
	
	SET_SEGV_LOCATION();
	if( ( !ProfileServ.enable && !cmdparams->channel ) || ( !ProfileServ.enableprofilechan && cmdparams->channel ) )
		return NS_SUCCESS;
	if( ValidateNick( cmdparams->av[0] ) == NS_FAILURE ) 
	{
		profile_report( cmdparams, "Invalid Nickname Specified" );
		return NS_SUCCESS;
	}
	/* only netadmins may delete others Profiles */
	if (ircstrcasecmp(cmdparams->av[0], cmdparams->source->name) && cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN )
	{
		profile_report( cmdparams, "You may only delete your own Profile." );
		return NS_SUCCESS;
	}
	pu = ns_calloc( sizeof( ProfileUser ) );
	strlcpy( nicklower, cmdparams->av[0], MAXNICK );
	if( DBAFetch( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) ) == NS_FAILURE )
	{
		profile_report( cmdparams, "No Profile exists for specified nickname" );
	}
	else
	{
		pfs_delete_profile ( cmdparams, pu );
	}
	ns_free( pu );
	return NS_SUCCESS;
}

/*
 * Delete Nicknames Profile
 *
 * removes passed ProfileUser
 * if cmdparams is NULL then delete called
 * from expire procedure
*/
void pfs_delete_profile ( CmdParams *cmdparams, ProfileUser *pu )
{
	ProfileData *pd;
	
	pd = ns_calloc( sizeof( ProfileData ) );
	strlcpy( pd->typenick, "RNT" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "BDT" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "AGE" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "SEX" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "MST" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "LOC" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "HOB" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "GT1" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "GT2" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "GT3" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "GT4" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	strlcpy( pd->typenick, "GT5" , PFS_TYPENICKSIZE );
	strlcat( pd->typenick, nicklower, PFS_TYPENICKSIZE );
	DBADelete( "UserData", pd->typenick );
	DBADelete( "UserList", nicklower );
	if (cmdparams)
		profile_report( cmdparams, "Profile for %s deleted.", pu->nick );
	ns_free( pd );
	return;
}

/*
 * Expire Old Profiles Timer
*/
int pfs_expire_profiles(void) 
{
	SET_SEGV_LOCATION();
	if( ProfileServ.expiredays > 0 )
		DBAFetchRows( "UserList", expireoldprofiles);
	return NS_SUCCESS;
}

/*
 * Expire Old Profiles
*/
int expireoldprofiles(void *data, int size) 
{
	ProfileUser *pu;

	pu = ns_calloc( sizeof ( ProfileUser ) );
	os_memcpy( pu, data, sizeof ( ProfileUser ) );
	if( ( me.now - pu->lastseen ) > ( ProfileServ.expiredays * TS_ONE_DAY ) )
		pfs_delete_profile ( NULL, pu );
	ns_free( pu );
	return NS_FALSE;
}

/*
 * User Mode Change
*/
int pfs_UmodeUser (CmdParams *cmdparams)
{
	ProfileUser *pu;
	
	SET_SEGV_LOCATION();
	if (cmdparams->source->user->Umode & UMODE_REGNICK)
	{
		pu = ns_calloc( sizeof( ProfileUser ) );
		strlcpy( nicklower, cmdparams->source->name , MAXNICK );
		if( DBAFetch( "UserList", strlwr(nicklower), ( void * )pu, sizeof( ProfileUser ) ) != NS_FAILURE )
		{
			/* Profile Exists so update last seen time */
			strlcpy( pu->nick, cmdparams->source->name , MAXNICK );
			pu->lastseen = me.now;
			DBAStore( "UserList", nicklower, ( void * )pu, sizeof( ProfileUser ) );
		}
		ns_free( pu );
	}
	return NS_SUCCESS;
}
