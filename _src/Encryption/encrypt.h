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
#include <string>
#include <string.h>

#define ENCRYPT_CMD_LEN 97
#define CHECKSUM_ENCRYPT 100

char c_original[ENCRYPT_CMD_LEN] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890,./<>?;:\'\"[]\\{}|=-+_)(*&^%$#@!~` ";
char c_encrypt [ENCRYPT_CMD_LEN] = "yr(GRMJ1#\'Fme]Kc3<0`{QCBa,$pDx2[h;g8_./unU|+fELqYN~}7l>=dzX?WkjTVH%@b6s9viIo4:v5Aw&O*tP!\\S^)Z\" -";

std::string encryptCMD(std::string text);
std::string unencryptCMD(std::string text);

std::string encryptCMD( std::string text ) {
	for (unsigned int i = 0; i <= text.length(); i++) {
		for (unsigned int x = 0; x <= ENCRYPT_CMD_LEN; x++) {
			if (text[i] == c_original[x]) {
				text[i] = c_encrypt[x];
				break;
			}
		}	
	}
	return text;
}

std::string unencryptCMD( std::string text ) {
	for (unsigned int i = 0; i <= text.length(); i++) {
		for (unsigned int x = 0; x <= ENCRYPT_CMD_LEN; x++) {
			if (text[i] == c_encrypt[x]) {
				text[i] = c_original[x];
				break;
			}
		}
	}
	return text;
}