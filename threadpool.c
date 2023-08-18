
#include "threadpool.h"
#include <string.h>
#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#define MAXT_IN_POOL 200
////////////////////////////////////////////FUNCTIONS SOS///////////////////////////////////////////////////
void printErrors(int poolsize,int maxNumJobs,int typeError){//0 wrong command, 1 System Call Failed 
	if(typeError==0)
		printf("Usage: threadpool <%d> <%d>\n",poolsize,maxNumJobs);
	else{
		perror("error: System Call Failed !!!\n");
	}
}

////////////////////////////////////////////INITIALIZATIONS///////////////////////////////////////////////
int functionDispatch_fn(void* parameter){
	int *par=(int*)parameter;
	for(int i=0;i<3;i++){
		printf("index: %d\n",*par);
		sleep(1);
	}
	return *par;
}
typedef int (*dispatch_fn)(void *);

threadpool* create_threadpool(int num_threads_in_pool){//this is the counstructor of the threadpool
	if(num_threads_in_pool>MAXT_IN_POOL || num_threads_in_pool<0){
		printErrors(num_threads_in_pool,0,0);
		return NULL;
	}
	threadpool* threadPoolCreated=(threadpool*)malloc(sizeof(threadpool));
	if(threadPoolCreated==NULL){
		printErrors(0,0,1);
		return NULL;
	}
	
	threadPoolCreated->num_threads=num_threads_in_pool;
	threadPoolCreated->threads=(pthread_t*)malloc(num_threads_in_pool*sizeof(pthread_t));
	if(threadPoolCreated->threads==NULL)
		printErrors(0,0,1);

	threadPoolCreated->qsize=0;
	threadPoolCreated->qhead=NULL;
	threadPoolCreated->qtail=NULL;
	pthread_mutex_init(&threadPoolCreated->qlock,NULL);
	pthread_cond_init(&threadPoolCreated->q_not_empty,NULL);
	pthread_cond_init(&threadPoolCreated->q_empty,NULL);
	threadPoolCreated->shutdown=0;
	threadPoolCreated->dont_accept=0;

	int i;
	int fine;
	for(i=0;i<num_threads_in_pool;i++){
		fine = pthread_create(&threadPoolCreated->threads[i], NULL, do_work,threadPoolCreated); 
		if (fine){ 
			printErrors(num_threads_in_pool,threadPoolCreated->qsize,1); 
			return NULL;
		} 
	}
	return threadPoolCreated;
}
/////////////////////////////////////****************************//////////////////////////////////////


void creatingList(threadpool* from_me, dispatch_fn dispatch_to_here, void *arg){

	pthread_mutex_lock((&from_me->qlock));
	if(from_me->dont_accept==1){
		pthread_mutex_unlock((&from_me->qlock));
		return;
	}

	if(from_me->qhead==NULL && from_me->qtail==NULL){

		
		from_me->qtail=(work_t*)malloc(sizeof(work_t));
		if(from_me->qtail==NULL){
			printErrors(0,0,1);
		}
		from_me->qtail->routine=dispatch_to_here;
		from_me->qtail->arg=arg;
		from_me->qtail->next=NULL;
		from_me->qhead=from_me->qtail;
		from_me->qsize++;
		pthread_cond_signal(&from_me->q_not_empty);
	}
	else{
		work_t* neWork =(work_t*)malloc(sizeof(work_t));
		if(neWork==NULL){
			printErrors(0,0,1);
		}
		neWork->routine=dispatch_to_here;
		neWork->arg=arg;
		neWork->next=NULL;

		from_me->qhead->next=neWork;
		from_me->qhead=neWork;
		from_me->qsize++;
		pthread_cond_signal(&from_me->q_not_empty);
	}
	pthread_mutex_unlock((&from_me->qlock));
}

void dispatch(threadpool* from_me, dispatch_fn dispatch_to_here, void *arg){

	if(from_me==NULL){
		printErrors(0,0,0);
		return;
	}
	
	creatingList(from_me,dispatch_to_here,arg);
		
}
////////////////////////////////////////*************************////////////////////////////////////

void* do_work(void* p){
	threadpool* threadpoolCreated=(threadpool*)p;
	work_t* temp;
	while(1){
		
		pthread_mutex_lock(&threadpoolCreated->qlock);
		if(threadpoolCreated->shutdown==1){
			pthread_mutex_unlock(&threadpoolCreated->qlock);
			return NULL;
		}

		if(threadpoolCreated->qsize==0){
			pthread_cond_wait(&threadpoolCreated->q_not_empty,&threadpoolCreated->qlock);
			if(threadpoolCreated->shutdown==1){
				pthread_mutex_unlock(&threadpoolCreated->qlock);
				return NULL;
			}
		}
		if(threadpoolCreated->qsize!=0){
			if(threadpoolCreated->qtail==NULL && threadpoolCreated->qhead!=NULL){
				temp=threadpoolCreated->qhead;
				threadpoolCreated->qhead=threadpoolCreated->qhead->next;
				threadpoolCreated->qsize--;
			}
			else{
				temp=threadpoolCreated->qtail;
				threadpoolCreated->qtail=threadpoolCreated->qtail->next;
				threadpoolCreated->qsize--;
			}

			if(threadpoolCreated->qsize==0 && threadpoolCreated->dont_accept==1){//
				pthread_cond_signal(&threadpoolCreated->q_empty);
			}
			pthread_mutex_unlock(&threadpoolCreated->qlock);
			temp->routine(temp->arg);
			free(temp);
			temp=NULL;
		}
	}	
}

void destroy_threadpool(threadpool* destroyme){
	int t,rc;

	pthread_mutex_lock(&destroyme->qlock);
	destroyme->dont_accept=1;
	if(destroyme->qsize>0)
		pthread_cond_wait(&destroyme->q_empty,&destroyme->qlock);//recive signal when the list is empty
	destroyme->shutdown=1;
	pthread_cond_broadcast(&destroyme->q_not_empty); 
	pthread_mutex_unlock(&destroyme->qlock);
	
	for(t=0;t<destroyme->num_threads;t++) { 
		rc = pthread_join(destroyme->threads[t],NULL); 
		if (rc) { 
			printErrors(destroyme->num_threads,destroyme->qsize,1); 
			return;
		}
	}
	free(destroyme->threads);
	destroyme->threads=NULL;
	pthread_mutex_destroy(&destroyme->qlock);
	pthread_cond_destroy(&destroyme->q_not_empty);
	pthread_cond_destroy(&destroyme->q_empty);
	free(destroyme);
	destroyme=NULL;
}

/*int main(int argc, char *argv[]) { 

	int argv1=atoi(argv[1]);//number of threads
	int argv2=atoi(argv[2]);//number of tasks
	printf("THE NUMBERS ARE: %d %d\n",argv1,argv2);
	threadpool* newThread=create_threadpool(argv1);

	int *arrTemp=(int*)malloc(argv2*sizeof(int));
	int i;
	for(i=0;i<argv2;i++){
		arrTemp[i]=i;
		dispatch(newThread,functionDispatch_fn,arrTemp+i);
	}

	destroy_threadpool(newThread);

	newThread=NULL;
	free(arrTemp);
	arrTemp=NULL;
	

	return 0;
}*/