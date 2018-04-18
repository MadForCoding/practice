

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>


typedef struct room{
    char* name;
    char** connections;
    char* room_type;
    int num_con;
    int curr_index;
} room;



/************************************************
 * Function: get_names
 * Description: Gets seven random names out of the hardcoded array of names. Puts
 *    these names into their own array, and also reads them into the room structs
 * Parameters: struct room rooms[7] - an array of room structs
 *             char * picked_names[7] - an array to stick the names into
 *             char * array_of_names[10] - the 10 hardcoded room names
 * *********************************************/

void get_names(struct room rooms[7], char * picked_names[7], char * array_of_names[10]){

   int j, i, status;
   char holder[16];

   /* This is a weird for-loop, in that we want to use a counted loop to take
    * advantage of the iterator variable, but we only want to increment the loop
    * if the string is successfully entered into the array.
    */
   for(j = 0; j < 7; ){

       strcpy(holder, array_of_names[rand()%10]);	//Get a random name

       status = 0;

       while(status == 0){			//Loop until the status var is set

	   for(i = 0; i < 7; i++){	//Check each name so far to make sure it's not a duplicate 
	       if(strcmp(picked_names[i], holder) == 0){
		   status = 1;		//Note: status=1 does not mean success here. It means we're done looking and should repeat with a different name.
	       }
	   }
	   if(status == 0){		//We didn't find the name - good to add it
	       strcpy(picked_names[j], holder);
	       strcpy(rooms[j].name, holder);
	       j++;			//Now we want to increment the loop
	       status = 1;
	   }
       }

   }

}




/************************************************
 * Function: get_types
 * Description: Assigns types to the room of structs. Randomly assigns start and end
 *    rooms and then makes the rest mid rooms
 * Parameters: struct room rooms[7] - the array of room structs
 * *********************************************/

void get_types(struct room rooms[7]){

    int start_index = 0;
    int end_index = 0;
    int i = 0;

    //Randomly generate tow numbers 0-6, and make sure they're different
    do {
	start_index = rand() % 7;
	end_index = rand() % 7;
    } while (start_index == end_index);

    //Assign the start and end rooms based off the random numbers
    strcpy(rooms[start_index].room_type, "START_ROOM");
    strcpy(rooms[end_index].room_type, "END_ROOM");

    //All the rest of the rooms are mid rooms
    for (i = 0; i < 7; i++) {
	if (i != start_index && i != end_index) {
	    strcpy(rooms[i].room_type, "MID_ROOM");
	}
    }

}



/************************************************
 * Function: get_connected
 * Description: Quite the function we have here... This is where the magic happens. Essentially, I started out not assigning
 *    connections in the sane way that Brewster showed us, so I had to come up with this to make it work.
 *    Iterates through all the room structs, and For each of the connections (the number of which there were
 *    were defined earlier), it picks a random room to connect to. It checks that we are able to add a room and that
 *    this wouldn't create a loop back to the initial room. If all of the checks are met, it adds the connection
 *    and then finds the connecting room and makes it connect bak. No one way passages for us.
 * Parameters: struct room rooms[7] - the array of room structs
 *             char * names[7] - the names array, for easier access
 * *********************************************/

void get_connected(struct room rooms[7], char * names[7]){

    //Yes, all these iterator variables are necessary.
    int i = 0;
    int j = 0;
    int k = 0;
    int n = 0;
    int status = 0;
    int index = 0;

    char holder[16];

    for (n = 0; n < 7; n++) {		//For each of the rooms in the struct array...

	for(i = 0; i < rooms[n].num_con; ) {	//For each of the connections...

	    status = 0;
	    index = rand () % 7;

	    //Get the first room's name
	    strcpy(holder, names[index]);

	    while(status == 0){		//While we haven't been told to break out...

		for(j = 0; j < rooms[n].num_con; j++) {

		    //Check if the connection already exists or if the connection would be a loop on the current room
		    if(strcmp(holder, rooms[n].name) == 0 || strcmp(holder, rooms[n].connections[j]) == 0) {
			status = 1;
		    }

		}

		//If there's not room to add another connection, this room is full - break out and go to the next room
		if(rooms[n].curr_index >= rooms[n].num_con || rooms[index].curr_index >= rooms[index].num_con) {
		    i++;
		    break;
		}

		//Otherwise, you want to make the connection that you've been testing for the whole time. Yay!
		else if(status == 0) {

		    //Actually make the dang connection official in this room
		    strcpy(rooms[n].connections[rooms[n].curr_index], holder);
		    rooms[n].curr_index++;

		    //Find the room you're connecting to, and make it connect back
		    for(k = 0; k < 7; k++){		
			if(strcmp(rooms[k].name, holder) == 0){
			    strcpy(rooms[k].connections[rooms[k].curr_index], rooms[n].name);
			    rooms[k].curr_index++;
			}		
		    }

		    i++;	//Go to the next room

		}
	    }

	}
	
    }



}


/************************************************
 * Function: timekeeping
 * Description: This is the function that the timekeeping thread
 *    calls. Gets the time, writes it to a file, and then
 *    reads it back from that file and displays it to the user.
 * Parameters: void * argunemt - whatever is passed by the call
 * *********************************************/
void * timekeeping (void * argument) {

    int result_code = 1;

    //Get the argument and cast it
    pthread_mutex_t * timelock = (pthread_mutex_t *) argument;
    
    //Should hang up the thread until the game thread unlocks the mutex
    result_code = pthread_mutex_lock(timelock); //Hangs here

    time_t curr_time;
    struct tm * time_info;
    char time_buffer[80];
    char file_buffer[80];
    FILE * time_file;

    //Get the time and stick it into the file
    time_file = fopen("./currentTime.txt", "w");
    time(&curr_time);
    time_info = localtime(&curr_time);
    strftime(time_buffer, 80, "%I:%M%p, %A, %B %d, %Y\n", time_info);
    fprintf(time_file, time_buffer);
    fclose(time_file);

    //REad in the time from the file and display it
    time_file = fopen("./currentTime.txt", "r");
    fgets(file_buffer, 80, time_file);
    fprintf(stdout, "\n%s\n", file_buffer);

    pthread_exit(NULL);


}



/************************************************
 * Function: main
 * Description: Where the magic happens. Makes the structs, does file I/O, and
 *    plays the game. Also deals with threads.
 * Parameters: None
 * *********************************************/

int main(){

    //Define all the variables of your wildest dreams
    int i = 0;
    int j = 0;
    int steps = 0;
    int doneForReal = 0;
    int done = 0;
    int index = 0;
    int indexConnection = 0;

    pid_t pid;

    FILE * file;

    char buffer[12];
    char inputBuffer[512];
    char directory_path[64];
    char location[32];
    char * names[7];
    char* possible_names[10] = {"reinhardt", "dva", "roadhog", "zarya", "winston", "junkrat", "ana", "mercy", "lucio", "zenyatta"};	//Our array of possible names
    char ** steps_thru_maze;

    struct room rooms[7];

    int result_int;		//For the thread and mutex
    pthread_t myThreadID;
    pthread_mutex_t lock;

    srand(time(NULL));		//RNG seed


    /*
     * In this chunk: setup for the thread stuff.
     * Spawn a mutex and lock it, then spawn the second thread.
     * The second thread will try to lock that same mutex, and end up hanging.
     * Resource: thegeekstuff.com/2012/05/c-mutex-examples/?refcom
     */
    
    //Init mutex and check for error
    if (pthread_mutex_init(&lock, NULL) != 0) {
	fprintf(stderr, "Couldn't init mutex\n");
	exit(1);
    }

    //If init was successful, lock in main thread
    //This allows the time thread to hang until the game thread unlocks the variable.
    pthread_mutex_lock(&lock);

    //Start 2nd thread (that tries to lock) and check for errors
    result_int = pthread_create(&myThreadID, NULL, timekeeping, &lock);
    if (result_int != 0) {
	fprintf(stderr, "Couldn't start the thread\n");
    }
    

    /*
     * In this chunk: Initalize the different arrays you'll need to use.
     */

    //Kick things off by giving yourself a simple array to hold the seven random room names.
    for (i = 0; i < 7; i++) {
	names[i] = malloc(sizeof(char) * 16);
    }

    //Now, the slightly trickier memory allocation for the array of structs.
    //This one even has a 2D array of strings to hold the connections.
    for (i = 0; i < 7; i++) {
	rooms[i].name = malloc(sizeof(char) * 16);
	rooms[i].room_type = malloc(sizeof(char) * 16);
	rooms[i].curr_index = 0;

	//Here is where we specify that each room must have 3-6 connections.
	do {
	    rooms[i].num_con = (rand() % 6) + 1;
	} while (rooms[i].num_con < 3);

	//Here's that 2D array. So cool!
	rooms[i].connections = malloc(sizeof(char *) * rooms[i].num_con);
	for(j = 0; j < rooms[i].num_con; j++){
		rooms[i].connections[j] = malloc(sizeof(char) * 16);
	}
    }

    //The last array to initialize is the one that'll hold the path the user takes.
    //To be entirely truthful, 200 is an arbitrary number. This limit can be changed. 
    steps_thru_maze = malloc(sizeof(char *) * 200);
    for (i = 0; i < 200; i++) {
	steps_thru_maze[i] = malloc(sizeof(char) * 16);
    }


    /*
     * In this chunk: The initial setup of the room structs.
     * After this bit, we'll never need to use the possible_names array again,
     * because we'll have picked names.
     */

    //These are all functions described above.
    get_names(rooms, names, possible_names);
    get_types(rooms);
    get_connected(rooms, names);


    /*
     * In this chunk: Creating the directory that we'll stick the files in.
     */

    //Get the PID and stick it on the end of the hardcoded directory path
    pid = getpid();
    sprintf(directory_path, "olesona.rooms.%d", pid);

    //Create the directory, and give everyone all the permissions.
    //In this case, user, group, and others can all read, write, and execute
    //Do check for an error though, just in case.
    if ( (mkdir(directory_path, S_IRWXU | S_IRWXG | S_IRWXO) ) == -1) {
	fprintf(stderr, "Couldn't open directory \n");
	exit(1);
    }


    /*
     * In this chunk: Writing to the files that make up the rooms. We open the files
     * in the directory that we just created for writing to, and then put in all
     * the required info.
     */

    for (i = 0; i < 7; i++) {		//For each room...

	//Frankenstein together a string ("location") to represent the file path to this particular room
	sprintf(location, "%s/%s", directory_path, rooms[i].name);

	if ( (file = fopen(location, "w+")) != NULL) {		//We're good

	    //Note: fprintf is much nicer than write(). I tried write() before. Not recommended.
	    fprintf(file, "ROOM NAME: %s\n", rooms[i].name);

	    //Stick the connections into the file as well
	    for (j = 0; j < rooms[i].num_con; j++) {
		if ( (strcmp(rooms[i].connections[j], "")) != 0) {
		    fprintf(file, "CONNECTION %d: %s\n", (j + 1), rooms[i].connections[j]);
		}
		memset(rooms[i].connections[j], '\0', 16);	//so we can read into it later
	    }

	    //Add on the room type
	    fprintf(file, "ROOM TYPE: %s\n", rooms[i].room_type);

	    //Now, blank out the struct fields, so that we can read back into them.
	    //Also close the file so we don't have to deal with weird multiple opening errors
	    memset(location, '\0', 32);
	    memset(rooms[i].name, '\0', 16);
	    memset(rooms[i].room_type, '\0', 16);
	    rooms[i].num_con = 0;
	    rooms[i].curr_index = 0;
	    fclose(file);

	}

	else {		//There was an error opening the file. Fail miserably.
	    fprintf(stderr, "Error opening file");
	    exit(1);
	}

    }


    /*
     * In this chunk: Reading from the files back into the array of structs.
     * It's not the prettiest way of doing it, but at least it avoids strtok(). I tried
     * to do this with strtok and got really weird bugs (e.g. Having my entire array
     * get cleared at random times). So, this approach, while hardcode-y, seems
     * to be working much better.
     */

    for (i = 0; i < 7; i++) {	//For each of the room names we picked...

	//String Frankenstein makes a comeback	
	sprintf(location, "./%s/%s", directory_path, names[i]);

	if ( (file = fopen(location, "r")) != NULL ) {		//We're good

	    //Get the name of the room by manipulating the file pointer
	    if (fseek(file, 11, SEEK_SET) == -1) {	//Error
		exit(1);
	    }
	    if (fgets(rooms[i].name, 16, file) == NULL) {	//Error
		exit(1);
	    }
	    //If you made it here, you're good!
	    //Stick a null terminator on the end of that to make it a proper cstring.
	    rooms[i].name[strlen(rooms[i].name) - 1] = '\0';

	    //Now, for the connections.
	    //Use a buffer that checks for the CONNECTION string, and go from there.
	    fgets(buffer, 11, file);

	    while(strcmp(buffer, "CONNECTION") == 0){	//If it's what you're looking for...
		//go to the right place, and get the name.
		//fgets reads unitl the newline, so you should be good.
		fseek(file, 4, SEEK_CUR);
		fgets(rooms[i].connections[rooms[i].curr_index], 16, file);
		rooms[i].connections[rooms[i].curr_index][strlen(rooms[i].connections[rooms[i].curr_index]) - 1] = '\0';	//Convoluted way of saying "slap a null terminator on it"

		//Now, update your variables and get ready for the next loop through.
		rooms[i].curr_index++;
		rooms[i].num_con++;
		memset(buffer, 0, 12);
		fgets(buffer, 11, file);

	    }


	    //So close! Only the room type left.
	    //Get there, read it in, and slap a null terminator on it.
	    fseek(file, 1, SEEK_CUR);
	    fgets(rooms[i].room_type, 16, file);
	    rooms[i].room_type[strlen(rooms[i].room_type) - 1] = '\0';
	    memset(buffer, 0, 12);

	}

	else {		//Something broke. Rest in pieces
	    exit(1);
	}

	fclose(file);	//If you got here, you're done! Good job.

    }


    /*
     * In this chunk: The setup for the game! Finally!
     * Print intro text and identify the start room, then set up your other vars.
     */

    fprintf(stdout, "\nYou find yourself in a maze...\n");

    for (i = 0; i < 7; i++) {		//Find the start room and remember where it is
	if (strcmp(rooms[i].room_type, "START_ROOM") == 0) {
	    index = i;
	    break;
	}
    }

    done = 1;	//This will keep track of whether we won yet. Set to 1 so the loop works right.
    indexConnection = 0; 	//Used to remember where to put the next room in steps_thru_maze
    steps = 0;		//To keep track of how many moves it takes


    /*
     * In this chunk: The game itself.
     * Nested do-while loops allow for easy printing of the first room in the array.
     * The outer one prints stuff, while the inner one deals with user input.
     */

    do{
	doneForReal = 0;

       	/*This do-while loop handles user input. Prompt the user to select a room,
	 * and move to that room if you can (incrementing tracker variables
	 * appropriately). If you can't move to that room, give an error and reprompt.
	 */
	
	do {

	    //First, print out the room to the screen.
	    fprintf(stdout, "\nCURRENT ROOM: %s\n", rooms[index].name);
	    fprintf(stdout, "POSSIBLE CONNECTIONS: ");
	    for (i = 0; i < rooms[index].num_con; i++) {
		//Print different pucntuation based on which connection it is.
		if ((strcmp(rooms[index].connections[i], "") != 0) && (i != rooms[index].num_con - 1)) {
		    fprintf(stdout, "%s, ", rooms[index].connections[i]);
		}
		else if(i == (rooms[index].num_con - 1)){
		    fprintf(stdout, "%s.\n", rooms[index].connections[i]);
		}
	    }
	    
	    //Prompt the user for input
	    fprintf(stdout, "WHERE TO? >");
	    memset(inputBuffer, 0, 512);	//Blank your buffers
	    memset(buffer, 0, 12);
	    fgets(inputBuffer, 512, stdin);	//fgets is nice and hangs until the user hits enter
	    inputBuffer[strlen(inputBuffer) - 1] = '\0';

	    //Check if the user wants the time. If so, unlock the mutex, let the thread do its thing, and re-init.
	    if (strcmp("time", inputBuffer) == 0) {
		pthread_mutex_unlock(&lock);
		pthread_join(myThreadID, NULL);

		/*
		 * The next chunk of code is supposed to re-init all the mutex stuff
		 * so that 'time' can be entered multiple times. I couldn't get it
		 * to work (the mutex wouldn't re-lock and would just hang).
		 * I suspect it has something to do with the lock variable getting 
		 * passed to the thread and possibly overwritten.
		 * Anyways, the time command will work exactly once each time you run
		 * the program.
		 */
		//Relock the mutex and restart the thread
		//pthread_mutex_lock(&lock);
		/*result_int = pthread_create(&myThreadID, NULL, timekeeping, &lock);
		if (result_int != 0) {
		    fprintf(stderr, "Couldn't start the thread\n");
		}*/

	    }

	    //Else, check if they entered a room name
	    else {

		//Now, check if the input is valid (a connected room name)
		for (i = 0; i < rooms[index].num_con; i++) {

		    if (strcmp(rooms[index].connections[i], inputBuffer) == 0 && indexConnection < 200) {
			//Yay! It's valid. Step to it.
			strcpy(buffer, rooms[index].connections[i]);
			strcpy(steps_thru_maze[indexConnection], buffer);
			doneForReal = 1;
			steps++;
			indexConnection++;
			break;
		    }

		}

	    }

	    //If you got to this point and i broke out of the for loop on its own, the input wan't valid. Tell the user to try again.
	    if (i == rooms[index].num_con && strcmp("time", inputBuffer) != 0) {
		fprintf(stdout, "\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	    }
	    

	} while (doneForReal == 0);

	/*
	 * In this chunk: "Moving" to a different room and checking if you've won.
	 * Change the index variable to the new room, and see if you're a winner.
	 * Print a message if so; else, redo the whole process.
	 */
	
	//"Move" to the next room by updating the index var
	for (i = 0; i < 7; i++) {
	    if (strcmp(buffer, rooms[i].name) == 0) {
		index = i;
	    }
	}

	//Check if you've won yet
	if (strcmp(rooms[index].room_type, "END_ROOM") == 0) {

	    //Edge case: You only took 1 step and then won. Print the right sentence.
	    if(steps == 1){
		fprintf(stdout, "YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICOTRY WAS: \n", steps);
	    }

	    //More likely case: you took multiple steps
	    else{
		fprintf(stdout, "YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS: \n", steps);
	    }

	    //Finally, print out the steps they took to get there.
	    for(i = 0; i < steps; i++){
		fprintf(stdout, "%s\n", steps_thru_maze[i]);
	    }
	    done = 0;
	
	}
    
    } while (strcmp(rooms[index].room_type,"END_ROOM") != 0 && done == 1);

    return 0;

}

//We did it, America.
