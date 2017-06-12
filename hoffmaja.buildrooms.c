#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>


void shuffle(int array[], int length) {
	int i, j, temp;
	for (i = 0; i < length; i++) {
		j = (rand() % 10);
		temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}

int main() {
	
	srand(time(NULL));
	
	struct Room {
		char name[32];
		int nameIndex;
		int connectionsIndex;
	};
	
	char* roomNames[10];					// ID by first letter
	roomNames[0] = "Bedroom"; 				// 66
	roomNames[1] = "Kitchen"; 				// 75
	roomNames[2] = "Closet"; 				// 67
	roomNames[3] = "Dining Room"; 			// 68
	roomNames[4] = "Mail Room"; 			// 77
	roomNames[5] = "Secret Lab"; 			// 83
	roomNames[6] = "Horrific B-Movie Room"; // 72
	roomNames[7] = "Foyer"; 				// 70
	roomNames[8] = "Laundry Room"; 			// 76
	roomNames[9] = "Office"; 				// 79
	
	int connections[7][7];
	memset(connections, 0, sizeof(connections)); // This works but row * column doensn't
	int i, j, nConnections = -1, rand_index = -1;
	
	// Create connections
	for (i = 0; i < 7; i++) {
		nConnections = rand();
		nConnections = nConnections % 4 + 3;
		for (j = 0; j < nConnections; j++) {
			rand_index = rand();
			rand_index = rand_index % 7;
			if (rand_index != i /* Cannot equal itself */ ) {
				connections[i][rand_index] = 1;
				connections[rand_index][i] = connections[i][rand_index];
			}
		}
	}
	
	// Array of randomly ordered indexes 
	int randIndexes[10], ri;
	for (ri = 0; ri < 10; ri++) {
		randIndexes[ri] = ri;
	}
	
	shuffle(randIndexes, 10);
	
	struct Room rooms[7];
	int r, name_index, connections_index;
	for (r = 0; r < 7; r++) {
		struct Room newRoom;
		// Seven of ten rooms selected using randomly ordered indexes
		newRoom.nameIndex = randIndexes[r];
		// Add name
		strcpy(newRoom.name, roomNames[newRoom.nameIndex]);
		// Connections assigned in order, but randomly generated so still random
		newRoom.connectionsIndex = r;
		// Add to rooms array
		rooms[r] = newRoom;
	}
	
	
	
	char *roomName = "ROOM NAME:";
	char *connection = "CONNECTION";
	char *roomType = "ROOM TYPE:";
	
	char filename[80];
	char buffer[20];
	char pid[10];
	FILE *fp;
	int f;
	
	char directoryName[80] = "./hoffmaja.rooms.";
	sprintf(pid, "%d/", getpid());
	strcat(directoryName, pid);
	
	int res = mkdir(directoryName, 0755);
	
	for (f = 0; f < 7; f++) {
		// Create filename
		strcpy(directoryName, "./hoffmaja.rooms."); // Reset directory name, shoul prob change this
		strcat(directoryName, pid);
		strcpy(filename, roomNames[rooms[f].nameIndex]);
		strcat(filename, ".rooms.");
		sprintf(buffer, "%d", getpid());
		strcat(filename, buffer);
		strcat(directoryName, filename);
		
		fp = fopen(directoryName, "w");
		fprintf(fp, "%s %s\n", roomName, roomNames[rooms[f].nameIndex]);
		
		int c, connectionNum = 1;
		int *connectionsRow = connections[rooms[f].connectionsIndex];
		for (c = 0; c < 7; c++) {
			if (connectionsRow[c]) {
				fprintf(fp, "%s %d: %s\n", connection, connectionNum, rooms[c].name);
				connectionNum++;
			}
		}
		
		if (f == 0) {
			fprintf(fp, "%s %s\n", roomType, "START_ROOM");
		} else if (f == 6) {
			fprintf(fp, "%s %s\n", roomType, "END_ROOM");
		} else {
			fprintf(fp, "%s %s\n", roomType, "MID_ROOM");
		}
		
		fclose(fp);
	}

}