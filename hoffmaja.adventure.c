// Jason Hoffman
// Program 2, CS 344
// May 12, 2017

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

// Struct to represent rooms
struct Room {
	char name[100];
	int id;
	int position;
	int numOutboundConnections;
	char *outboundConnections[6];
};


int countLines(FILE *fp);
struct Room parseRoom(FILE *fp, int lines);
void exploreRooms(struct Room rooms[7]);
char * removeNewline(char *str);
struct Room nextRoom(struct Room rooms[7], struct Room room, char *name);
void * fileThread();
void printTime();
void * timeFile();

// Multi-threading variables
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t print_thread;
pthread_t file_thread;
int time_return;
int file_return;


int main() {
    
    // Most of this first part is taken from our Unit 2 slides
	int newestDirTime = -1; // Modified timestamp of newest subdir examined
	char targetDirPrefix[32] = "hoffmaja.rooms."; // Prefix we're looking for
	char newestDirName[256]; // Holds the name of the newest dir that contains prefix
	memset(newestDirName, '\0', sizeof(newestDirName));
    
    // Call lock on mutex
    pthread_mutex_lock(&myMutex);
    file_return = pthread_create(&file_thread, NULL, timeFile, NULL);


	DIR *dirToCheck; // Holds the directory we're starting in
	struct dirent *fileInDir; // Holds the current subdir of the starting dir
	struct stat dirAttributes; // Holds information we've gained about subdir
	
	struct Room Rooms[7];
	
	dirToCheck = opendir("."); // Open up the directory this program was run in
	
	if (dirToCheck > 0) {// Make sure the current directory could be opened
		while ((fileInDir = readdir(dirToCheck)) != NULL) { // Check each entry in dir
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) {// If entry has prefix
			
				printf("Found the prefex: %s\n", fileInDir->d_name);
				stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry
				
				if ((int)dirAttributes.st_mtime > newestDirTime) { // If this time is bigger
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}

	closedir(dirToCheck); // Close the directory we opened

	DIR *dirToRead = opendir(newestDirName); // Directory object from newest directory
	FILE *fp; // Filestream 
	struct dirent *roomFile; // dirent to read from directory
	int lines;  
	
    
	int roomIndex = 0; // Index for use while parsing files
	while ((roomFile = readdir(dirToRead)) != NULL ) {
		if (roomFile->d_name[0] > 46) { // Ignore current and parent (. ..)
			char fullPath[100];
			sprintf(fullPath, "%s/%s", newestDirName, roomFile->d_name);  // Create full path for filestream
			fp = fopen(fullPath, "r"); // Open for reading
			lines = countLines(fp); // Get number of lines in file
			rewind(fp); // Rewind for parseRooms function
			Rooms[roomIndex] = parseRoom(fp, lines); // Add parsed file too Rooms array
			roomIndex++; // Increment
		}
	}
	
    // After all files are parsed, we begin the "game"
	exploreRooms(Rooms);
}  // END OF MAIN

// Helper function to count the lines in a file
int countLines(FILE *fp) {
	int c, l = 0;
	do {
		c = fgetc(fp);
		if (c == 10) {
			l++; } 
	} while (c != EOF);
    
	return l;
}

// Function to parse a room file to create Room struct
struct Room parseRoom(FILE *file, int lines) {
	struct Room room;
	char line[100];
	int lineCount = 1, connection = 0;
	while (fgets(line, sizeof(line), file) != NULL) {
        // If first or last line get room name or position
		if (lineCount == 1 || lineCount == lines) {
			memcpy(line, (line+11), strlen(line));
			if (lineCount == 1) {
				strcpy(room.name, removeNewline(line));
				room.id = line[0];
			} else {
                // START_ROOM, MID_ROOM, END_ROOM IDed by first letter
				if (line[0] == 83) {
					room.position = 0; // Start
				} else if (line[0] == 69) {
					room.position = 2; // End
				} else {
					room.position = 1; // Mid
				}
			}
		} else {
            // For all other lines in the file add the
            // outbound connections in order.
			memcpy(line, (line+14), strlen(line));
			room.outboundConnections[connection] = malloc(strlen(line) + 1); // Have to create the memory space first
			removeNewline(line);
			strcpy(room.outboundConnections[connection], line);
			connection++;
		}
		lineCount++;
	}
    // Number of outbound connections is lines minus first and last
	room.numOutboundConnections = lines - 2;
	return room;
}

// Function to facilitate moving through rooms
// Takes array of Room structs. Always size 7
void exploreRooms(struct Room rooms[7]) {
    
	struct Room currentRoom;
	int i;
	for (i = 0; i < 7; i++) { // Find first room
		if (rooms[i].position == 0) {
			currentRoom = rooms[i];
			break;
		}
	}
	
	int steps = 0; // int to track turns
	struct Room exploredRooms[50]; // Array for explored rooms
    memset(exploredRooms, '\0', 50 * sizeof(struct Room *));
	size_t bufferSize = 0;
	char *lineEntered = NULL;
    
	while (1) { // Loop until exit
		printf("\nCURRENT LOCATION: %s\n", currentRoom.name);
		printf("POSSIBLE CONNECTIONS: ");
		int c;
        // Loop through outbound connections and print them
		for (c = 0; c < currentRoom.numOutboundConnections; c++) {
			if (c == currentRoom.numOutboundConnections - 1) {
                 // Special case for last room to show period
                printf("%s.", removeNewline(currentRoom.outboundConnections[c]));
				break;
			} 
			printf("%s, ", removeNewline(currentRoom.outboundConnections[c]));
		}
		printf("\nWHERE TO? >");
        // Get user input then use nextRoom function to get next
        // room based on input
		getline(&lineEntered, &bufferSize, stdin);
        
        if (strcmp("time\n", lineEntered) == 0) {
            // Based on Piazza pseudocode
            // I don't think I have it right
            pthread_mutex_unlock(&myMutex);
            pthread_join(file_thread, NULL);
            pthread_mutex_lock(&myMutex);
            printTime();
        } else {
            currentRoom = nextRoom(rooms, currentRoom, lineEntered);
        }
        
        // If still in the same room don't take a step
        if (!strcmp(currentRoom.name, lineEntered)) {
            exploredRooms[steps] = currentRoom;
            steps++;
        }

        // END_ROOM IDed by position == 2 from struct
        if (currentRoom.position == 2) {
            printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
            printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
            int j;
            for (j = 0; j < steps; j++) {
                printf("%s\n", exploredRooms[j].name);
            }
            exit(0); // Exit with code 0
        }
        
	}
}


// Function to get next room based on user input
struct Room nextRoom(struct Room rooms[7], struct Room room, char *name) {
	
	char *errMsg = "HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n";
	int c, i;
    struct Room newRoom;
    // Loop through connections. If user input matches one of the connections
    // Then get matching struct and return it
	for (c = 0; c < room.numOutboundConnections; c++) {
		if (strcmp(room.outboundConnections[c], removeNewline(name)) == 0) {
			for (i = 0; i < 7; i++) {
				if (strcmp(rooms[i].name, name) == 0) {
					newRoom = rooms[i];
					return newRoom;
				}
			}
		}
	}
    
    // If no match found return original room and print error message
    printf("\n%s", errMsg);
    return room;
}

void * timeFile() {
    pthread_mutex_lock(&myMutex);
    FILE *fp;
    fp = fopen("currentTime.txt", "w");
    char outstr[200];
    time_t t;
    t = time(NULL);
    strftime(outstr, sizeof(outstr), NULL, localtime(&t));
    fputs(outstr, fp);
    fclose(fp);
		usleep(100);
    pthread_mutex_unlock(&myMutex);
    return NULL;
}


void printTime() {
    // Blocks itself while file is created on other thread
    FILE *fp;
    char timeBuffer[200];
    fp = fopen("currentTime.txt", "r");
    fgets(timeBuffer, sizeof(timeBuffer), fp);
    printf("\n%s\n", timeBuffer);
}

// Helper function to remove a new line from character strings
char * removeNewline(char *str) {
    // Search string for newline character
	char *p = strchr(str, '\n');
    // If found change to a terminator
	if (p != NULL) *p = '\0';
	return str;
}
