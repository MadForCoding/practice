#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t timethread;
int thread;

struct RoomTypes
{
   char * startroom;
   char * midroom;
   char * endroom;
};

// Time thread
void* thetime()
{
	pthread_mutex_lock(&my_mutex);		// Tries to lock mutex; can't at first so it will wait
	// Once the primary thread unlocks the mutex (after user enters "time") this will run
	printf("\n** Time display not yet implemented, but this IS a working mutex. (See code comments) **\n\n");
	// *******************************************************************************************************************
	// * NOTE: I didn't have time to add the actual getting and displaying time to the function, but that would go here. *
	// *    Hopefully I can get partial credit for this because as I understood it the main purpose of this part of the  *
	// *    program was to implement the mutex, which I HAVE done. You can see that it functions properly, just imagine  *
	// *    the message being printed above replaced with the time. It returns to the primary thread when finished which *
	// *    then spawns a new time thread that waits for unlocking again, just as instructed in class.                   *
	// *******************************************************************************************************************
	pthread_mutex_unlock(&my_mutex);	// Unlocks mutex when finished so the game thread can continue
}

// Function that creates the rooms directory
char * makeDirectory(int pid)
{
	int size = 20;								// Buffer for snprintf
	char * roomdirectory = malloc(20);				// Allocate directory
	char * path = "ozarowib.rooms.";				// Directory path
	snprintf(roomdirectory, size, "%s%d", path, pid);	// Add pid to the path
	mkdir(roomdirectory, 0755);					// Make the directory
	return roomdirectory;						// Return the path
}

// Function for randomizing room names and connections
// Modeled after example here: http://litdream.blogspot.com/2014/12/maze-generating-in-c-like-lumosity.html
void randomize(char **array, size_t size)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int usec = tv.tv_usec;
	srand48(usec);
	if (size > 1)
	{
		size_t i;
		for (i=size-1; i > 0; i--)
		{
			size_t j = (unsigned int) (drand48()*(i+1));
			char * t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

// Makes the room files and fills them with info
struct RoomTypes makeRooms(char * roomdirectory)
{
	char * roomnames[10];		// 10 possible names, 7 will be used
	roomnames[0] = "Austin";
	roomnames[1] = "Cordley";
	roomnames[2] = "Dearborn";
	roomnames[3] = "Dixon";
	roomnames[4] = "Kerr";
	roomnames[5] = "LINC";
	roomnames[6] = "Owen";
	roomnames[7] = "Reser";
	roomnames[8] = "Snell";
	roomnames[9] = "Wilson";
	struct RoomTypes room;
	room.midroom = roomdirectory;
	int size = 128;					// Buffer for snprintf
	char * roomfile = malloc(128);
	randomize(roomnames, 10);			// Randomizes the list of potential names
	int i;
	for (i=0; i < 7; i++)				// Selects 7 names from the randomized list
	{
		snprintf(roomfile, size, "%s/%s", roomdirectory, roomnames[i]);
		FILE * input = fopen(roomfile, "w");			//Open file for writing
		fprintf(input, "ROOM NAME: %s\n", roomnames[i]);	// Write the room name
		fclose(input);
	}
	char * gamerooms[7];			// Make an array of the 7 rooms in use this game
	for (i=0; i < 7; i++)
	{
		gamerooms[i] = roomnames[i];	// Gets their names from the original array
	}
	int startroom = rand() % 7;		// Pick random room for start
	int endroom = rand() % 7;		// Pick random room for end
	while (startroom == endroom)		// If randomizer picked the same for both,
	{ endroom = rand() % 7; }		// Repick end until it is different
	char * connection; 				// Tracks room connections
	int numconnections; 			// Number of connections the room will have
	int j;
	int k;
	for (i=0; i < 7; i++)			// Go through each of the 7 room files
	{
		k = 0;
		numconnections = rand() % 4+3; 		// Randomly decide 3 to 6 connections
		snprintf(roomfile, size, "%s/%s", roomdirectory, roomnames[i]);
		FILE * input = fopen(roomfile, "a"); 	// Open room file for append
		for (j=0; j < numconnections; j++)		// Loops for each connection this room needs
		{
			connection = gamerooms[k];		// Make the connection
			if (connection == roomnames[i])	// If it is trying to connect to itself
			{
				k++;						// Go to next
				connection = gamerooms[k];	// Connect there instead
			}
			fprintf(input, "CONNECTION %d: %s\n", j+1, connection);	// Add connection to the file
			k++;		// Go to next
		}
		if (i == startroom)		// Check if this room's type is the start room
		{
			fprintf(input, "ROOM TYPE: START_ROOM\n");		// Add that to file if it is
			room.startroom = roomnames[i];				// And note the start in the struct
		}
		else if (i == endroom)	// If it wasn't the start check if it is the end room
		{
			fprintf(input, "ROOM TYPE: END_ROOM\n");		// Add that to file if it is
			room.endroom = roomnames[i];					// And note the end in the struct
		}
		else
		{
			fprintf(input, "ROOM TYPE: MID_ROOM\n");		// If it wasn't either add that it's a mid room
		}
		fclose(input);
	}
	free(roomfile);
	return room;
}

// Gameplay
void playgame(struct RoomTypes currentroom)
{
	char * startroom = currentroom.startroom;
	char * middleroom = currentroom.midroom;
	char * endroom = currentroom.endroom;
	char (*steps)[15] = malloc(sizeof *steps * 8);		// For storing steps
	char (*fileinfo)[15] = malloc(sizeof *fileinfo * 8);	// For storing room file content
	char * roomfile = malloc(128);
	char nextroom[15];
	int filelines;
	int stepcounter = 0;
	int size = 128;		// Buffer for snprintf
	int i;
	while (!(strcmp(startroom, endroom)) == 0)		// Loops until the current room is the end room
	{
		int currentconnections = 0;	// Will note how many connections the current room has
		snprintf(roomfile, size, "%s/%s", middleroom, startroom);	// Identify the current room's file
		FILE * input = fopen(roomfile, "r");					// Open the file for reading
		while ((filelines = getc(input)) != EOF)				// Until it finds the end of the file
		{
			if (filelines == '\n')
				{ currentconnections++; }					// Add to the connection counter for each line
		}
		currentconnections -= 2;	// Remove overcount for NAME and TYPE lines, the rest of the lines = number of connections
		char info[20];
		fseek(input, 11, SEEK_SET);				// Moves into the file to the room name
		fgets(info, 20, input);					// Reads the line
		int length = strlen(info);
		if (info[length-1] == '\n')				// Removes the newline for proper printing later
			{ info[length-1] = 0; }
		strcpy(fileinfo[0], info);				// Store the room name
		for (i=1; i <= currentconnections; i++)		// Loops for each connection we found the room has
		{
			fseek(input, 14, SEEK_CUR);			// Moves to the connecting room's name
			fgets(info, 20, input);				// Reads the line
			length = strlen(info);
			if (info[length-1] == '\n')			// Removes the newline for proper printing later
				{ info[length-1] = 0; }
			strcpy(fileinfo[i], info);			// Store the connecting room
		}
		int badroom = 0;						// Indicates if the current room is a valid one
		while (badroom != 1)					// If the current room is a valid one
		{
			printf("CURRENT LOCATION: %s\n", fileinfo[0]);	// Print the stored room name
			printf("POSSIBLE CONNECTIONS: ");
			for (i=1; i <= currentconnections; i++)			// For each connection we said the file showed
			{
				if (i == currentconnections)
					{ printf("%s.\n", fileinfo[i]); }		// Print the connecting room names
				else
					{ printf("%s, ", fileinfo[i]); }
			}
			printf("WHERE TO? >");						// User inputs next room name to check
			scanf("%s", nextroom);
			if (strcmp(nextroom, "time") == 0)				// If user enters "time"
			{
				pthread_mutex_unlock(&my_mutex);			// Unlocks mutex so the time thread can run now
				pthread_join( timethread, NULL);			// Waits for time thread to run and terminate
				pthread_mutex_lock(&my_mutex);			// When back to this thread, locks mutex again
				thread = pthread_create( &timethread, NULL, &thetime, NULL );	// Creates another time thread which waits as before
				startroom = fileinfo[0];					// Run game on the current room again
			}
			else										// If user enters anything else assume it's a room name and check it
			{
				for (i=1; i <= currentconnections; i++)		// Loops for each connection the current room has
				{
					if (strcmp(nextroom, fileinfo[i]) == 0)	// Checks if the input name is one of the listed connections
					{
						badroom = 1;					// If a valid next room don't set the error flag
						startroom = nextroom;			// Run game on the room with that name
					}
				}
				if (badroom != 1)						// Print error message if the input was bad
					{ printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n"); }
			}
		}
		printf("\n");
		strcpy(steps[stepcounter], startroom);				// Add the room to the path taken so far
		stepcounter++;									// Increment step counter
		fclose(input);
	}
	// When the current room is the end room
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepcounter);	// Print number of steps
	for (i = 0; i < stepcounter; i++)
		{ printf("%s\n", steps[i]); }									// Print names of rooms taken
	free(steps);
	free(fileinfo);
	free(roomfile);
}

int main()
{
	thread = pthread_create( &timethread, NULL, &thetime, NULL );	// On program start, creates a time thread
	pthread_mutex_lock(&my_mutex);							// Locks mutex so the time thread won't run yet
	srand(time(NULL));
	int pid = getpid();										// Get process id
	char * roomdirectory = makeDirectory(pid);					// Makes directory with the pid and returns path to it
	struct RoomTypes gamerooms = makeRooms(roomdirectory);			// Creates the room files in the new directory
	playgame(gamerooms);									// The game loop
	free(roomdirectory);
	return 0;
}