#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// PREPROCESSOR CONSTANT DEFINITIONS
#define MAX_OUT 6
#define MIN_OUT 3
#define NUM_ROOMS 7

// Declare Mutex and thread handles
pthread_mutex_t locks[2];
pthread_t threads[2];

// struct used to store room information
struct room {
  char name[15];
  char outgoing[6][15];
  int out_count;
  int room_type;
};

// enum to easily differentiate room type
enum room_type { start = 0, mid, end };

// FUNCTION DEFINITIONS - See Implementaion Below Main for Descriptions
void* doWriteTimeThread(void *close);
void* doReadTimeThread(void *close);
int getGameDir(const char *rootdir, char* buffer);
int fillRooms(const char *gamedir, struct room *rooms, int *start_room);
int indexofRoom(const struct room *rooms, int count, const char *val);
int indexof(char names[6][15], int count, const char *val);

// MAIN FUNCTION
int main() {
  // allocate space for NUM_ROOMS struct room's
  struct room *rooms = malloc(sizeof(struct room)*NUM_ROOMS);
  int i = 0,              //general purpose loop counter
      success = 0,        //used to handle function return values
      current_room,       //index of current room in array of struct rooms
      rooms_visited[100], //array of indices in struct rooms array for locations visited
      visit_count = 0,    //count the number of rooms visited
      close = 0;          //flag to threads that its time to break execution and join

  char buffer[50],        //general purpose buffer for strings
       *readBuffer = NULL,//buffer for getline user input
       *c;                //char pointer for removing newline chars from strings
  size_t readSize = 0;    //size in bytes of malloc call by getline

  // Initialize both mutexes and aquire lock for main thread
  for (i = 0; i < 2; i++) {
    if(pthread_mutex_init(&locks[i], NULL) != 0){
      printf("Failed to initialize mutex %d\n", i+1);
      free(rooms);
      return 1;
    }
    pthread_mutex_lock(&locks[i]);
  }
  // Create thread to track time and write to file.
  // pass in the close flag to communicate when its time to finish execution
  // assert that thread creation is successfull, else exit
  success = pthread_create(&threads[0], NULL, &doWriteTimeThread, &close);
  if (success != 0) {
    printf("Failed to create thread: %s\n", strerror(success));
    free(rooms);
    return 1;
  }
  // Create thread to read time from file and write to stdout.
  // pass in the close flag to communicate when its time to finish execution
  // assert that thread creation is successfull, else exit
  success = pthread_create(&threads[1], NULL, &doReadTimeThread, &close);
  if (success != 0) {
    printf("Failed to create thread: %s\n", strerror(success));
    free(rooms);
    return 1;
  }
  // read in directories in execution path and return the name of
  // the most recent rooms directory in buffer
  // return value of -1 if no rooms directory exists
  success = getGameDir("./", buffer);
  if (success == -1) {
    printf("No Room Directory!\n");
    return 1;
  }
  // fill the array of struct rooms and initialize the current_room with the
  // startroom
  // return value of -1 for failure to open files.
  success = fillRooms(buffer, rooms, &current_room);
  if (success == -1) {
    printf("Error Reading Room Files!\n");
    return 1;
  }
  // add current_room to rooms visited and increment count
  rooms_visited[visit_count++] = current_room;

  do { //while (rooms[current_room].room_type != end)
    // success stores INT_MAX if time command ran
    // indicates that Current Location and Connections should be left out this round
    // ELSE, print out prompt
    if (success != INT_MAX) {
      printf("CURRENT LOCATION: %s\n", rooms[current_room].name);
      printf("POSSIBLE CONNECTIONS:");
      // Loop through current rooms outgoing connections and print
      for (i = 0; i < rooms[current_room].out_count; i++) {
        // print comma for all rooms in list except the last which gets period
        if (i+1 != rooms[current_room].out_count)
          printf(" %s,", rooms[current_room].outgoing[i]);
        else
          printf(" %s.", rooms[current_room].outgoing[i]);
      }
    }
    printf("\nWHERE TO? >");
    // get user input with getline. Getline will dynamically allocate char array
    success = getline(&readBuffer, &readSize, stdin);
    c = strchr(readBuffer, '\n'); // get pointer to final newline
    *c = '\0';                    // replace with null terminator
    //if time command is called
    if (strcmp(readBuffer, "time") == 0) {
      // Unlock mutex for write thread
      pthread_mutex_unlock(&locks[0]);
      usleep(50); // brief sleep to give thread a chance to gain lock
      // lock mutex to block write thread from executing another loop
      pthread_mutex_lock(&locks[0]);
      // unlock mutex for read thread
      pthread_mutex_unlock(&locks[1]);
      usleep(50); // brief sleep to allow thread to gain lock
      // lock mutex for read thread to prevent another loop
      pthread_mutex_lock(&locks[1]);
      success = INT_MAX;  // INT_MAX signals the proper prompt following time command
    }
    // check if specified room is not in connections
    else if(indexof(rooms[current_room].outgoing, rooms[current_room].out_count, readBuffer) < 0)
      printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    // room is in connections move to the room
    else {
      // get index of room in struct rooms array
      success = indexofRoom(rooms, NUM_ROOMS, readBuffer);
      // room not found continue
      if (success < 0) continue;
      current_room = success; //change current_room to new room
      rooms_visited[visit_count++] = success; // add room to visited and increment counter
      printf("\n"); //print formatting newline;
    }
    // get readBuffer and readSize ready for another getline call
    free(readBuffer);
    readBuffer = NULL;
    readSize = 0;

    //break loop when end room is found
  } while (rooms[current_room].room_type != end);

  // end room found print victory message
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", visit_count-1);

  // list off every step in path to end room
  for (i = 1; i < visit_count; i++)
    printf("%s\n", rooms[rooms_visited[i]].name);

  // set close flag for threads
  close = 1;
  // unlock each threads mutex
  pthread_mutex_unlock(&locks[0]);
  pthread_mutex_unlock(&locks[1]);

  // call join on each thread to wait for them to end
  for (i = 0; i < 2; i++)
    if (pthread_join(threads[i], NULL) != 0)
      perror("Failed Thread Join\n");
  // destroy the mutexes
  pthread_mutex_destroy(&locks[0]);
  pthread_mutex_destroy(&locks[1]);

  // free rooms from memory and return success
  free(rooms);
  return 0;
}

/*******************************************************************************
 ** Thread: doWriteTimeThread
 ** Arguments: void ptr, takes integer value used as flag for ending execution
 ** Description: Time Keeping Thread. Sits at lock until called upon by main
 **              thread to get the current time and write it to currentTime.txt
 ** Preconditions: mutex initialized and locked by main thread.
 ** Postconditions: file currentTime.txt left in execution directory.
 *******************************************************************************/
void* doWriteTimeThread(void *close) {
  int *i_close = (int*)close; // close flag
  char buffer[50];            // string buffer
  time_t rawtime;             // store raw time
  struct tm *timeinfo;        // store local time paramaters
  FILE *timeFile;             // file stream for writing
  // loop until close flag set
  while (!*i_close) {
    // lock at mutex until main thread unlocks
    pthread_mutex_lock(&locks[0]);
    // if flag is set break and join
    if (*i_close)
      break;
    // open file stream currentTime.txt for writing, print if error
    timeFile = fopen("./currentTime.txt", "w");
    if (timeFile == NULL) {
      perror("Error\n");
    }
    // file opened, get current time and write to file;
    else {
      // get rawtime
      time ( &rawtime );
      // use raw time to fill struct tm
      timeinfo = localtime ( &rawtime );
      // fill buffer with formatted current time
      strftime(buffer, 50, "%I:%M%p, %A, %B %d, %Y", timeinfo);
      // make AM/PM lowercase
      buffer[5] = tolower(buffer[5]);
      buffer[6] = tolower(buffer[6]);
      // remove leading 0 from single digit times
      if (timeinfo->tm_hour < 10 || (timeinfo->tm_hour > 12 && timeinfo->tm_hour < 22))
        memmove(buffer, buffer+1, strlen(buffer));
      // write formatted time to file then close file
      fputs(buffer, timeFile);
      fclose(timeFile);
    }
    // unlock mutex and pause briefly for main thread to acquire lock
    pthread_mutex_unlock(&locks[0]);
    usleep(50);
  }
  return NULL;
}
/*******************************************************************************
 ** Thread: doReadTimeThread
 ** Arguments: void ptr, takes integer value used as flag for ending execution
 ** Description: Print Time Thread. Sits at lock until called upon by main
 **              thread to read the current time from file and print to stdout
 ** Preconditions: mutex initialized and locked by main thread.
 ** Postconditions: No effect on game state or file structure
 *******************************************************************************/
void* doReadTimeThread(void *close) {
  int *i_close = (int*) close;  // close flag
  char buffer[50];              // buffer for reading time
  FILE *timeFile;               // handle for file stream

  // loop until close flag is set by main thread
  while (!*i_close) {
    // lock until called upon by main thread
    pthread_mutex_lock(&locks[1]);
    // if flag is set when lock is acquired break loop and exit
    if (*i_close)
      break;
    // open currentTime file stream for reading, alert if error occurs
    timeFile = fopen("./currentTime.txt", "r");
    if (timeFile == NULL) {
      perror("Error\n");
    }
    // file opened successfully
    else {
      // read current time into buffer
      fgets(buffer, 50, timeFile);
      // print to stdout
      printf("\n%s\n", buffer);
      // close file
      fclose(timeFile);
    }
    // unlock mutex and briefly sleep so main thread can acquire lock
    pthread_mutex_unlock(&locks[1]);
    usleep(50);
  }
  return NULL;
}

/*******************************************************************************
 ** Function: getGameDir
 ** Arguments: const char pointer to path of execution directory
 **            buffer to write the latest room directory to
 ** Description: gets the stats of each rooms directory and returns
 **              the name of the most recent in buffer
 ** Preconditions: buildrooms program has executed and rooms directory exists
 ** Postconditions: returns name of most recent rooms directory.
 *******************************************************************************/
int getGameDir(const char *rootdir, char *buffer) {
  int i = -1, // stores time of most recent rooms dir,
      // if it is still -1 after loop then no rooms directory exists
      j = 0;  // stores time of current rooms dir. used to compare to current largest
  struct stat dirStat;  // stores file stats
  DIR *dir;             // Directory stream
  struct dirent *dirEnt;// stores directory entry

  // open running directory
  dir = opendir(rootdir); // open directory
  if (dir == NULL) {
    perror("Directory Error:\n");
    return -1;
  }
  // read each directory entry
  while ((dirEnt = readdir(dir))) {
    // check if name begins with chenw5.buildrooms
    if (strncmp(dirEnt->d_name, "chenw5.buildrooms", 17) == 0) {
      // get stats of directory
      stat(dirEnt->d_name, &dirStat);
      j = (int)dirStat.st_mtime;          // store lats modified time
      if (j > i) {                        // compare to current max
        strcpy(buffer, dirEnt->d_name);   // if greater copy dir name to buffer
        i = j;                            // j is new max
      }
    }
  }
  closedir(dir);                          // close directory
  return i;                               // -1 if no rooms dir found
}

/*******************************************************************************
 ** Function: fillRooms
 ** Arguments: const char pointer to path of rooms directory
 **            array of struct room to store room information in
 **            pointer to int to store start_room index.
 ** Description: Reads each file from the rooms directory and fills a struct room
 **              for each one in the array. Sets start_room to the index of the
 **              room with "START_ROOM" roomtype.
 ** Preconditions: buildrooms program has run and getGameDir has retrieved the latest
 **                rooms directory name.
 ** Postconditions: array of struct room contains all information needed to play
 **                 the game.
 *******************************************************************************/
int fillRooms(const char *gamedir, struct room *rooms, int *start_room) {
  struct dirent *dirEnt;                //store directory entry
  DIR *dir;                             // directory stream handle
  FILE *file;                           // file stream handle
  char buffer[100];                     // char buffer for file text
  char *c;                              // char pointer to remove newlines
  int i = 0,                            // used to store index of ':' char
      room_count = 0;                   // keep track of room index

  // give each room an outgoing count of 0
  for (; room_count < NUM_ROOMS; room_count++)
    rooms[room_count].out_count = 0;
  room_count = 0;
  dir = opendir(gamedir);               // open gamedir
  if (dir == NULL) {
    perror("Directory Error\n");
    return -1;
  } 
  // read each file's information into a dir entry
  while ((dirEnt = readdir(dir))) {
    // ignore current and parent directory entries
    if (strlen(dirEnt->d_name) > 2){
      // store relative path from executing directory to file in buffer
      sprintf(buffer, "./%s/%s", gamedir, dirEnt->d_name);
      // open file for reading
      file = fopen(buffer, "r");
      // check that file opened successfully, if not return -1
      if (file == NULL ) { perror("File Error\n"); return -1;}
      // read in file line by line
      while(fgets(buffer, 100, file)!=NULL) {
        // remove newline from fgets
        c = strchr(buffer, '\n');
        *c = '\0';
        // get pointer to : char
        c = strchr(buffer, ':');
        // get index of colon by taking the offset from buffer
        i = c - buffer;
        // capture the room name
        if (strncmp(buffer, "ROOM NAME", i) == 0) {
          strcpy(rooms[room_count].name, c+2);
          // capture the connections
        }else if (strncmp(buffer, "CONNECTION", i-2) == 0) {
          strcpy(rooms[room_count].outgoing[rooms[room_count].out_count++], c+2);
          // capture the room type
        } else if (strncmp(buffer, "ROOM TYPE", i) == 0) {
          strcpy(buffer, c+2);
          if (strcmp(buffer, "END_ROOM")==0)
            rooms[room_count].room_type = end;
          else if (strcmp(buffer, "START_ROOM") == 0) {
            rooms[room_count].room_type = start;
            // set starting room
            *start_room = room_count;
          }
          else
            rooms[room_count].room_type = mid;
        }
      }
      // close file and increment room count
      fclose(file);
      room_count++;
    }
  }
  // close directory
  closedir(dir);
  return 0;
}

/*******************************************************************************
 ** Function: indexofRoom
 ** Arguments: array of struct room, count of rooms, value to compare
 ** Description: compare value to name of each room. return index of a match or
 **              -1 if no match found
 ** Preconditions: array of struct rooms filled
 ** Postconditions: No change to game state
 *******************************************************************************/
int indexofRoom(const struct room *rooms, int count, const char *val) {
  int i = 0;
  for (; i < count; i++)
    if (strcmp(rooms[i].name, val) == 0)
      return i;
  return -1;
}

/*******************************************************************************
 ** Function: indexof
 ** Arguments: 2D array of chars representing outgoing connections,
 **            count of outgoing connections , value to compare
 ** Description: compare value to name of each connection. return index of a match or
 **              -1 if no match found
 ** Preconditions: array of struct rooms filled
 ** Postconditions: No change to game state
 *******************************************************************************/
int indexof(char names[6][15], int count, const char *val) {
  int i = 0;
  for (; i < count; i++)
    if (strcmp(names[i], val) == 0)
      return i;
  return -1;
}
