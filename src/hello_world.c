#include <stdio.h>
#include <unistd.h> 
#include <pthread.h>

static void input_rgb(void) {
	// length
	int ln = 13;
	// define input var (*input):one letter (input): string
	char *input;
	// length 13 (real length: 12)
	input = (char *) malloc(sizeof(char) * ln);
	// get input in console
	fgets(input, ln, stdin);
	// remove line breaker
	input[strlen(input) - 1] = '\0';
	// define orginal
	char *org;
	// push
	org = input;
	/* define rgbs */
	int red = 0;
	int green = 0;
	int blue = 0;
	/* split , : first */
	// define piece
	char *piece;
	// first split
	piece = strtok(input, ",");
	// null check
	if(piece != NULL){
		//println
		printf("Red: %s\n", piece);
		// (int)piece
		red = atoi(piece);
		// http://ra2kstar.tistory.com/49 : Param is null
		piece = strtok(NULL, ",");
		if(piece != NULL){
			green = atoi(piece);
			printf("Green: %s\n", piece);
			piece = strtok(NULL, ",");
			if(piece != NULL){
				blue = atoi(piece);
				printf("Blue: %s\n", piece);
			}
		}
	}
}

void* do_loop(void *data)
{
    int i;

    int me = *((int *)data);
    for (i = 0; i < 10; i++)
    {
        printf("%d - Got %d\n", me, i);
        sleep(1);
    }
}

int main () {
	/*
    int       thr_id;
    pthread_t p_thread[3];
    int status;
    int a = 1;
    int b = 2;      
    int c = 3;      

    thr_id = pthread_create(&p_thread[0], NULL, do_loop, (void *)&a);
    thr_id = pthread_create(&p_thread[1], NULL, input_rgb, (void *)&a);
    thr_id = pthread_create(&p_thread[2], NULL, do_loop, (void *)&c);

    pthread_join(p_thread[0], (void **) &status);
    pthread_join(p_thread[1], (void **) &status);
    pthread_join(p_thread[2], (void **) &status);
    */

	// define name *name: one name, name: string name
	char *name;
	name = (char *) malloc(sizeof(char) * 13);
	fgets(name, 13, stdin);
	// remove n

	name[strlen(name) - 1] = '\0';

	printf("star-name: %c\n", *name);
	printf("name: %s\n", name);

	char *params;
	params = name;

	 for(;*name;name++)
        printf("%c %d\n", *name, *name);

    printf("params: %s\n", params);
    char *ptr;

    ptr = strtok(params, ",");

    printf("ptr: %s\n", params);
    //input_rgb();

    char *ptr;
    ptr = strtok(name,",");

    printf("name : %s\n", ptr);

    while(ptr != NULL){
    	printf("%s\n", ptr);
    	ptr = strtok(NULL,",");
    }
    printf("str : %s\n" , name);
    return 0;
}