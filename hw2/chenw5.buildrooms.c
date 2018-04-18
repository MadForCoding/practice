#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// PREPROCESSOR CONSTANT DEFINITIONS
#define MAX_OUT 6
#define MIN_OUT 3
#define NUM_ROOMS 7

// struct used to store room information
struct room {
  int name;
  int outgoing[MAX_OUT];
  int out_count;
  int room_type;
};

// enum to easily differentiate room type
enum room_type { start = 0, mid, end };

// FUNCTION DEFINITIONS - See Implementaion Below Main for Descriptions
int getRandomName(int takenNames[], int *count);
int contains(const int *arr, int count, int val);

// MAIN function
int main() {
  // Seed random generator
  time_t t;
  srand((unsigned)time(&t));
  // array of struct rooms
  struct room rooms[NUM_ROOMS];
  // FULL list of possible room names
  char roomNames[10][15] = {
        "Chen",
        "Wei",
        "Wee",
        "Kyoka",
        "Switch",
        "PSV",
        "NDS",
        "GBA",
        "SONY",
        "PSP"
      },
       // List of room types
       roomTypes[3][11] = {
         "START_ROOM",
         "MID_ROOM",
         "END_ROOM"
       },  
       // buffer for writing to files
       fileBuffer[50];

  pid_t pid = getpid();           //get process ID
  FILE *outFile;                  // File stream handle
  int i = 0,                      // loop counter
      j = 0,                      // loop counter
      takenRoomNames[NUM_ROOMS],  // room names already taken
      takenRoomCount = 0,         // count of rooms already taken
      endroom,                    // store random index of endroom
      startroom,                  // store random index of startroom
      connection;                 //

  // initialize name, out_count, and room_type
  for (; i < NUM_ROOMS; i++) {
    // get random index of a name not yet taken
    rooms[i].name = getRandomName((int*)&takenRoomNames, &takenRoomCount);
    // initialize starting out_count to 0 and room_type to -1
    rooms[i].out_count = 0;
    rooms[i].room_type = -1;
  }
  // get random index for startroom and assign that room to the start room
  startroom = rand() % NUM_ROOMS;
  rooms[startroom].room_type = start;
  // get random indices for end room until it doesnt match startroom
  endroom = rand() % NUM_ROOMS;
  while (endroom == startroom) { endroom = rand() % NUM_ROOMS; }
  // assign room to be of type endroom
  rooms[endroom].room_type = end;
  // assign the rest of the rooms to be mid room
  for (i = 0; i < NUM_ROOMS; i++)
    if (i != endroom && i != startroom)
      rooms[i].room_type = mid;

  // CREATE OUTGOING CONNECTIONS
  for (i = 0; i < NUM_ROOMS; i++) {
    // continue if this room's count greater than or equal to MIN_OUT
    while (rooms[i].out_count < MIN_OUT) {
      // if rooms out_count is less than min, get a random number of connectins
      connection = 1 + rand() % MAX_OUT;
      // ensure random connections is in range 3 <= connection <= 6
      while (connection < MIN_OUT) {connection = 1 + rand() % 6;}
      // subtract random number of connections from total connections already set
      connection -= rooms[i].out_count;
      // loop to set net connections for room
      for (j = 0; j < connection; j++) {
        // get random room for connection
        startroom = rand() % NUM_ROOMS;
        // make sure the room is not already in its connections and its not the current room
        while (contains(rooms[i].outgoing, rooms[i].out_count, rooms[startroom].name) ||
            rooms[startroom].name == rooms[i].name) {
          // if so, get another random index and try again
          startroom = rand() % NUM_ROOMS;
        }
        // add random index to rooms outgoin connections and increment out_count
        rooms[i].outgoing[rooms[i].out_count++] = rooms[startroom].name;
        // check if the newly connected room already has the current room in its connections
        if (!contains(rooms[startroom].outgoing, rooms[startroom].out_count, rooms[i].name))
          // if not add a connection from new room to current room and increment its out_count
          rooms[startroom].outgoing[rooms[startroom].out_count++] = rooms[i].name;
      }
    }
  }
  // WRITE ROOMS to FILES int chenw5.buildrooms.PID directory
  // create directory name in fileBuffer
  sprintf(fileBuffer, "chenw5.buildrooms.%d", (int)pid);
  // create directory and throw error if something goes wrong
  if (mkdir(fileBuffer, 0755) != 0) {
    perror("Error Making Directory:\n");
    return 1;
  }
  // Loop through rooms and create a room file for each.
  for (i = 0; i < NUM_ROOMS; i++) {
    // create relative path to room in fileBuffer
    sprintf(fileBuffer, "./chenw5.buildrooms.%d/%s", (int)pid, roomNames[rooms[i].name]);
    // open file stream for writing and throw error if something goes wrong
    outFile = fopen(fileBuffer, "w");
    if (outFile == NULL) {
      perror("Error opening file:\n");
      return 1;
    }
    // create ROOM NAME line to write to file in fileBuffer
    sprintf(fileBuffer, "ROOM NAME: %s\n", roomNames[rooms[i].name]);
    // write line to file stream
    fputs(fileBuffer, outFile);
    // loop over rooms outgoing connections and write each connection to file
    for (j = 0; j < rooms[i].out_count; j++) {
      // create formatted Conncetion line to write to file
      sprintf(fileBuffer, "CONNECTION %d: %s\n", j+1, roomNames[rooms[i].outgoing[j]]);
      // write line to file stream
      fputs(fileBuffer, outFile);
    }
    // create formatted line for ROOM TYPE
    sprintf(fileBuffer, "ROOM TYPE: %s\n", roomTypes[rooms[i].room_type]);
    // write line to file stream
    fputs(fileBuffer, outFile);
    // close file
    fclose(outFile);
  }
  return 0;
}

/*******************************************************************************
 ** Function: getRandomName
 ** Arguments: int array containing indexes to taken names
 **            pointer to int count of names already taken
 ** Description: returns random int in range 0 <= int <= 9 which is not already
 **              present in takenNames. random int is added to takenNames and
 **              taken count is incremented.
 ** Preconditions: None
 ** Postconditions: game directory found or invalid call to adventure (need buildrooms)
 *******************************************************************************/
int getRandomName(int takenNames[], int *count) {
  int random,
      found;
  // if no name is taken create random index, add to takenNames and return.
  if (count == 0) {
    takenNames[(*count)++] = rand() % 10;
    return takenNames[0];
  }
  // loop until a random index is found that is not in takenNames
  do {  //while (found)
    random = rand() % 10;             //generate random number
    // check if takenNames contains random
    found = contains(takenNames, *count, random);
    // loop until found is false
  } while (found);
  // add random index to takenNames and increment count
  takenNames[(*count)++] = random;
  // return index
  return random;
}

/*******************************************************************************
 ** Function: contains
 ** Arguments: pointer to integer array, count of integer array
 **            value to look for
 ** Description: loops over array checking if each element matches val
 **              returns 1 if found and 0 if not found
 ** Preconditions: None
 ** Postconditions: None
 *******************************************************************************/
int contains(const int *arr, int count, int val) {
  int i = 0;
  for (; i < count; i++)
    if (arr[i] == val)
      return 1;
  return 0;
}
