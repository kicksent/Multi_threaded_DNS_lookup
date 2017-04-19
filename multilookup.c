/***************************
File: Multilookup
Nick Trierweiler
**************************/

#include "util.h"
#include "queue.h"
#include "multilookup.h"

char * OUTPUT_FILE;
char errorstr[SBUFSIZE];
int NUM_FILES;
int num_queued;
int num_resolved;
queue q;
pthread_mutex_t output_lock;
pthread_mutex_t queue_lock;


void * read_file(void * f){
	//printf("inside read_file\n");
	FILE * file = fopen((char*) f, "r");
	if(!file){

	    sprintf(errorstr, "Error Opening Input File: %s", (char*)f);
	    return NULL;
	}
   	char hostname[MAX_NAME_LENGTH];
   	char * hostname_ptr; //if i dont do this then hostname gets turned into a pointer

	while(fscanf(file, INPUTFS, hostname) > 0){
		pthread_mutex_lock(&queue_lock); //LOCK	
		while(queue_is_full(&q)){
			pthread_mutex_unlock(&queue_lock);
			usleep(rand()%100);
			pthread_mutex_lock(&queue_lock);
		}
		hostname_ptr = malloc(sizeof(hostname));
		strcpy(hostname_ptr, hostname);
		queue_push(&q, hostname_ptr);
		num_queued++;
		//printf("pushed to queue: %s\n", hostname_ptr);
		pthread_mutex_unlock(&queue_lock); //UNLOCK
	}
	fclose(file);

	return NULL;
}
void * write_file(void * outputfp){
	char firstipstr[INET6_ADDRSTRLEN];
	char* hostname;
	usleep(100); //delay resolver threads
	while(1){
		if(num_resolved == num_queued){ //are we done?
			printf("num_queued: %d, num_resolved: %d\n", num_queued, num_resolved);
			return(NULL);
		}
		while(!queue_is_empty(&q)){
			pthread_mutex_lock(&queue_lock); //LOCK
			hostname = (char*) queue_pop(&q);
			num_resolved++;
			pthread_mutex_unlock(&queue_lock);

			/* from lookup.c */
			if(dnslookup(hostname, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE)
			{
				fprintf(stderr, "dnslookup error:, HOSTNAME %s\n", hostname); //alert user when hostname not found
				strncpy(firstipstr, "", sizeof(firstipstr));
			}
			/* Write to Output File */
			//printf("writing %s,%s to file\n", hostname, firstipstr);
			if (hostname != NULL){
				pthread_mutex_lock(&output_lock);
				fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
				//printf("writing to file: %s,%s\n", hostname, firstipstr);
				
				pthread_mutex_unlock(&output_lock);
			}
			free(hostname); //fixes valgrind direct loss
			
		}
	}	
	
	return NULL;
}

//credit to stackoverflow.com/questions/19232957/pthread-create-passing-an-integer-as-the-last-argument
//also stackoverflow.com/questions/32487579/why-does-a-function-accept-a-void-pointer
void * request_pool(void * argv){
	/* create a thread pool*/
	pthread_t request_threads[NUM_FILES];
	char ** files = (char**) argv;
	/* assign a thread to each file pointer */
	int i;
	for(i = 1; i < NUM_FILES+1; i++){
		pthread_create(&request_threads[i], NULL, read_file, (void*) files[i]);
	}
	for(i = 0; i< NUM_FILES; i++){
		pthread_join(request_threads[i], NULL);
	}
	return NULL;
}

void * resolve_pool(void* outputfp){
	pthread_t resolve_threads[NUM_FILES];
	int i = 0;
	for(i = 0; i < NUM_FILES; i++){
		pthread_create(&resolve_threads[i], NULL, write_file, outputfp);
		pthread_join(resolve_threads[i], NULL);
	}	
	return NULL;
}



int main(int argc, char* argv[]){
	queue_init(&q, 10);
	pthread_mutex_init(&output_lock, NULL);
	pthread_mutex_init(&queue_lock, NULL);
	
	/* seed random generator */
    srand(time(NULL));
    /* Local Vars */
    //FILE* inputfp = NULL;
    
    //char hostname[MAX_NAME_LENGTH]; moved to read_file
    char errorstr[MAX_NAME_LENGTH];
    //char firstipstr[INET6_ADDRSTRLEN];
    int i;

    /* Setup Local Vars for threads*/
    // pthread_t threads[NUM_THREADS];
    // int rc;
    // long t;
    // long cpyt[NUM_THREADS];
    
    /* Check Arguments */
    if(argc < MINARGS){
		fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
	return EXIT_FAILURE;
    }

    /* from lookup.c */
    OUTPUT_FILE = argv[argc -1];
    FILE* outputfp = fopen(OUTPUT_FILE, "w");
    if (!outputfp) {
        fprintf(stderr, "Error opening output file \n");
        return EXIT_FAILURE;
    }

    NUM_FILES = argc - 2;
    //printf("numfiles: %d", NUM_FILES); working
    
	for(i=1; i<(argc-1); i++){
		/* Get Input File Names for request */
		
		printf("file %s\n", argv[i]);
		if(!argv[i])
		{
		    sprintf(errorstr, "Error Grabbing Name of Input File: %s", argv[i]);
		    perror(errorstr);
		    break;
		}
	}

    //credit to code example cis.poly.edu/cs3224a/Code/ProducerConsumerUsingPthreads.c
    pthread_t pro, con;


    pthread_create(&pro, NULL, request_pool, (void *) argv);
    pthread_create(&con, NULL, resolve_pool, (void *) outputfp);



    /* wait for pro and con threads to terminate before exiting */
    pthread_join(pro, NULL);
    pthread_join(con, NULL);
    
    /*close file*/
    fclose(outputfp);

    /* clean up to prevent memory leak: locks, allocated memory */
    pthread_mutex_destroy(&queue_lock);
    pthread_mutex_destroy(&output_lock);


    return EXIT_SUCCESS;
}
