/* This file is part of the SDS 200A library project.
 *
 * It is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Libsds200a is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libsds200a. If not, see <http://www.gnu.org/licenses/>.
 *
 * (c) 2014 Simon Schuster, Sebastian Rachuj
 */

#include <stdio.h>

static int pstate = 0;
void state_toggle()
{
	if (pstate)
		putchar(' ');
	pstate = !pstate;
}

int main(int argc, char **argv)
{
	int state = 0;
	while (!feof(stdin) && !ferror(stdin)) {
		int cur = fgetc(stdin);
		if (cur == EOF)
			break;
		if (!state) {
			if (cur == '\'')
				state = 1;
			putchar(cur);
		} else {
			switch(cur) {
				case '0':
					printf("0000");
					state_toggle();
					break;
				case '1':
					printf("0001");
					state_toggle();
					break;
				case '2':
					printf("0010");
					state_toggle();
					break;
				case '3':
					printf("0011");
					state_toggle();
					break;
				case '4':
					printf("0100");
					state_toggle();
					break;
				case '5':
					printf("0101");
					state_toggle();
					break;
				case '6':
					printf("0110");
					state_toggle();
					break;
				case '7':
					printf("0111");
					state_toggle();
					break;
				case '8':
					printf("1000");
					state_toggle();
					break;
				case '9':
					printf("1001");
					state_toggle();
					break;
				case 'a':
					printf("1010");
					state_toggle();
					break;
				case 'b':
					printf("1011");
					state_toggle();
					break;
				case 'c':
					printf("1100");
					state_toggle();
					break;
				case 'd':
					printf("1101");
					state_toggle();
					break;
				case 'e':
					printf("1110");
					state_toggle();
					break;
				case 'f':
					printf("1111");
					state_toggle();
					break;
				case '\'':
					state = 0;
				default:
					pstate = 0;
					putchar(cur);
					break;
			}
		}
	}
}

