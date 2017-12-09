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

#include		"stdafx.h"
#include		"ghostlib.h"
#include		"resource.h"

#include		<fstream>
#include		<Windows.h>

#define			DB_NAME			"ghost.db"

std::vector		< GHOSTLIB > client_data;
int				i_res_code;

void ghostlib::addZombie(GHOSTLIB data) {
	client_data.push_back(data);
}

void ghostlib::deleteZombie(int idx) {
	client_data.erase(client_data.begin() + idx);
}

GHOSTLIB & ghostlib::getZombieData( int idx ) {
	return client_data[idx];
}

void ghostlib::updateZombieConnection(int idx, SOCKET sockref) {
	client_data[idx].socketRef = sockref;
}

int ghostlib::getZombieCount() {
	return client_data.size();
}

int ghostlib::getZombieIndex(SOCKET socketRef) {
	for (size_t i = 0; i < client_data.size(); i++) 
		if (client_data[i].socketRef == socketRef) 
			return i;
	return -1;
}

void ghostlib::parseZombie(SOCKET clientSocket, int zombieID, char * data) {
	client_data[zombieID].system_data = data;
}