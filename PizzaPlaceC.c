#include "p3140159-pizza2.h"
#include <pthread.h>

//mutexes
pthread_mutex_t cook_m;					//mutex gia paraskeuastes
pthread_mutex_t oven_m;					//mutex gia fournous
pthread_mutex_t deliverer_m;				//mutex gia dianomis
pthread_mutex_t screen_m;				//mutex gia othoni
pthread_mutex_t max_time_m;				//mutex gia megisto xrono oloklirosis
pthread_mutex_t max_cooling_time_m;			//mutex gia megisto xrono kriomatos
pthread_mutex_t cumulative_time_m;			//mutex gia sunoliko xrono oloklirosis
pthread_mutex_t cumulative_cooling_time_m;		//mutex gia sunoliko xrono kriomatos
pthread_mutex_t seed_m;					//mutex gia ton sporo

//conditions
pthread_cond_t cook_cond;				//cond gia paraskeuastes
pthread_cond_t oven_cond;				//cond gia fournous
pthread_cond_t deliverer_cond;				//cond gia dianomis

//variables
int cooks = NCOOK;					//plithos paraskeuaston
int ovens = NOVEN;					//plithos fournon
int deliverers = NDELIVERER;				//plithos dianomeon
int max_time = 0;					//megistos xronos oloklirosis paraggelias
int max_cooling_time = 0;				//megistos xronos kriomatos paraggelias
int cumulative_time = 0;				//sunolikos xronos oloklirosis paraggelion
int cumulative_cooling_time = 0;			//sunolikos xronos kriomatos paraggelion
int seed;						//timi sporou

//fuction called by the threads
void *order(void * customer){
	
	//variables used for the statistics
	struct timespec thread_start, pizza_baked, thread_finish;
	int time;
	clock_gettime(CLOCK_REALTIME, &thread_start);
	
	//order's id
	int id = (int) customer;
	int rc;
	int pizzas;
	int delivery_time;
	int cooling_time;
	
	//internal seed
	unsigned int temp_o;
	temp_o = seed;
	rc = pthread_mutex_lock(&seed_m);				
	seed = seed + 1;
	rc = pthread_mutex_unlock(&seed_m);
	if(rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}
	
  //COOKS JOB STARTS HERE
	
	//wait till at least one cook is available		
	pthread_mutex_lock(&cook_m);
	while(cooks==0){
		pthread_cond_wait(&cook_cond, &cook_m);	
	}
	cooks = cooks - 1;
	pthread_mutex_unlock(&cook_m);

	//number of pizzas
	pizzas = rand_r(&temp_o) % (NORDERHIGH - NORDERLOW) + NORDERLOW;
	
	//wait for pizza preparation
	sleep(TPREP * pizzas);

  //OVENS GET IN THE GAME
	pthread_mutex_lock(&oven_m);
	while(ovens==0){
		pthread_cond_wait(&oven_cond, &oven_m);
	}
	ovens = ovens - 1;
	pthread_mutex_unlock(&oven_m);
	
	//release the cook
	pthread_mutex_lock(&cook_m);
	cooks = cooks + 1;
	pthread_cond_signal(&cook_cond);
	pthread_mutex_unlock(&cook_m);

  //COOKS JOB FINISHES HERE
	
	//wait for pizza bake
	sleep(TBAKE);
	
	//get the timestamp when the pizzas are baked and waiting for the deliverer
	clock_gettime(CLOCK_REALTIME, &pizza_baked);

  //DELIVERERS DO YOUR THING	
	pthread_mutex_lock(&deliverer_m);
	while(deliverers==0){
		pthread_cond_wait(&deliverer_cond, &deliverer_m);	
	}

	//release the oven
	pthread_mutex_lock(&oven_m);
	ovens = ovens + 1;
	pthread_cond_signal(&oven_cond);
	pthread_mutex_unlock(&oven_m);

  //OVENS YOU'RE OUT
	deliverers = deliverers - 1;
	pthread_mutex_unlock(&deliverer_m);

	//delivery time
	delivery_time = rand_r(&temp_o) % (THIGH - TLOW) + TLOW;
	
	//wait for the deliverer to get to the customer
	sleep(delivery_time);

	//wait for the deliverer to return to the pizzeria
	sleep(delivery_time);

	//release the deliverer
	pthread_mutex_lock(&deliverer_m);
	deliverers = deliverers + 1;
	pthread_cond_signal(&deliverer_cond);
	pthread_mutex_unlock(&deliverer_m);

  //DELIVERERS' JOB DONE
	
	//get the finish timestamp and update cumulative time
	clock_gettime(CLOCK_REALTIME, &thread_finish);
	time = thread_finish.tv_sec - thread_start.tv_sec;
	pthread_mutex_lock(&cumulative_time_m);
	cumulative_time = cumulative_time + time;
	pthread_mutex_unlock(&cumulative_time_m);

	cooling_time = thread_finish.tv_sec - pizza_baked.tv_sec;	
	
	//update max and cumulative cooling times
	if(cooling_time > max_cooling_time){
		pthread_mutex_lock(&max_cooling_time_m);
		max_cooling_time = cooling_time;
		pthread_mutex_unlock(&max_cooling_time_m);
	}
	pthread_mutex_lock(&cumulative_cooling_time_m);
	cumulative_cooling_time = cumulative_cooling_time + cooling_time;
	pthread_mutex_unlock(&cumulative_cooling_time_m);

	//update max time	
	pthread_mutex_lock(&max_time_m);
	if(time>max_time){
		max_time = time;
	}	
	pthread_mutex_unlock(&max_time_m);
	
	//print the statistics	   
	rc = pthread_mutex_lock(&screen_m);
	printf("The order with number %d was delivered in %d minutes and was cooling for %d minutes. \n", id, time, cooling_time);
	rc = pthread_mutex_unlock(& screen_m);
	if(rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	return 0;
}

//main function
int main(int argc ,char * argv[]){

	//main variables
	int custNum;
	int rc;
	unsigned int temp;
	double average;	
	
	//main variables' check and initialization
	if(argc != 3) {
		printf("ERROR: the program should take two arguments, the number of customers and the seed!\n");
		exit(-1);
	}
	custNum = atoi(argv[1]);			//plithos pelaton kai metatropi se int
	seed = atoi(argv[2]);				//timi sporou kai metatropi se int

	//mutex initialization
	rc = pthread_mutex_init(&cook_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_mutex_init(&oven_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_mutex_init(&deliverer_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_mutex_init(&screen_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_mutex_init(&max_time_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_mutex_init(&max_cooling_time_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_mutex_init(&cumulative_time_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_mutex_init(&cumulative_cooling_time_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_mutex_init(&seed_m, NULL);
	if(rc != 0){
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc) ;
		exit(-1); 	
	}
	rc = pthread_cond_init(&cook_cond, NULL);
	if(rc != 0) {
    		printf("ERROR: return code from pthread_cond_init() is %d\n", rc) ;
       		exit(-1);
	}
	rc = pthread_cond_init(&oven_cond, NULL);
	if(rc != 0) {
 		printf("ERROR: return code from pthread_cond_init() is %d\n", rc) ;
       		exit(-1);
	}
	rc = pthread_cond_init(&deliverer_cond, NULL);
	if(rc != 0) {
 		printf("ERROR: return code from pthread_cond_init() is %d\n", rc) ;
       		exit(-1);
	}

	pthread_t threads_t[custNum];
	int id[custNum];
	
	//thread creation
	for(int i = 0 ; i < custNum ; i++){
		id[i] = i + 1;
		rc = pthread_create(&threads_t[i], NULL, order, (int *)id[i]);
		if(rc != 0){
			printf("ERROR: return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		//wait for the next order
		temp = seed;
		rc = pthread_mutex_lock(&seed_m);				
		seed = seed + 1;
		rc = pthread_mutex_unlock(&seed_m);		
		if(rc != 0) {	
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			pthread_exit(&rc);
		}
		sleep(rand_r(&temp) % (TORDERHIGH - TORDERLOW) + TORDERLOW);
	}	
	
	//thread join
	for(int i = 0 ; i < custNum ; i++){
		rc = pthread_join(threads_t[i], NULL);
		if(rc != 0){
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}

	//print statistics
	//max and average service times
	average = (double)cumulative_time / (double)custNum;
	printf("The average service time was: %.2f minutes \n", average);
	printf("The maximum service time was: %d minutes \n", max_time);
	//max and average cooling times
	average = (double)cumulative_cooling_time / (double)custNum;
	printf("The average cooling time was: %.2f minutes \n", average);
	printf("The maximum cooling time was: %d minutes \n", max_cooling_time);
	
	//mutex destruction
	rc = pthread_mutex_destroy(&cook_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&oven_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&deliverer_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&screen_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&max_time_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&max_cooling_time_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&cumulative_time_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&cumulative_cooling_time_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&seed_m);
	if(rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
 	rc = pthread_cond_destroy(&cook_cond);
	if(rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_cond_destroy(&oven_cond);
	if(rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_cond_destroy(&deliverer_cond);
	if(rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}

	//exit program
	exit(-1) ;
}	
