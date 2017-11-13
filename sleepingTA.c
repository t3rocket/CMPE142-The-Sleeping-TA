#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

pthread_mutex_t mutex_lock;
/*semaphore declarations */

sem_t students_sem; /* ta wait for a student to show up, student notifies ta his/her arrivial */

sem_t ta_sem; /** student wait for ta to help, ta notifies student he/she is read to help **/

/** the number of waiting students **/
int waiting_students = 0;

/* the maximum time (int seconds) to sleep */
#define MAX_SLEEP_TIME 3

/* number of potential students */
#define NUM_OF_STUDENTS 4

#define NUM_OF_HELPS 2

/* number of available seats */
#define NUM_OF_SEATS 2

volatile int seats[NUM_OF_SEATS] = {0};
volatile int empty_chair; 
volatile int next_stud;

int occupied=0;

void initialize()
{
	sem_init(&ta_sem,1,0);

	sem_init(&students_sem,1,0);
	pthread_mutex_init(&mutex_lock,NULL);
	empty_chair=0;
	next_stud=0;
}


void* teacher(void *arg)
{
	int help_time = 0;
	int v;
	printf("\nTA thread started");

	while(1){

		sem_wait(&students_sem); //TA waiting for student
		help_time = rand() % MAX_SLEEP_TIME + 1;

		pthread_mutex_lock(&mutex_lock);
		printf("\n\tTA has %d students waiting for help.",occupied);

		printf("\n\tTA helping student %d from seat #%d",seats[next_stud],(next_stud+1));
		occupied--;
		next_stud = (next_stud + 1)%NUM_OF_SEATS;

		pthread_mutex_unlock(&mutex_lock);
		sleep(help_time);
		printf("\n\tTA is avaiable for help.");
		sem_post(&ta_sem); //TA is free

	}
	pthread_exit(NULL);
}

void* student(void *thread_id)
{

	unsigned int id=(unsigned int)thread_id;
	int help_count=0;
	int v;
	int coding_time;
	int can_wait;
	printf("\n Started student %d",id);

	while(1)
	{


		printf("\nStudent %d coding ...",id);
		coding_time= rand() % MAX_SLEEP_TIME + 1;
		sleep(coding_time);
		printf("\nStudent %d going to TA for help #%d .",id,(help_count+1));


		pthread_mutex_lock(&mutex_lock);
		//printf("\noccupied=%d",occupied);
		if(occupied==NUM_OF_SEATS) //Check if all seats are occupied
		{
			printf("\nNo seat avaiable for student %d. Going back to coding.",id);
			pthread_mutex_unlock(&mutex_lock);
			continue;
		}
		else
		{
			seats[empty_chair]=id;
			occupied++;
			printf("\nStudent %d waiting for TA in seat #%d",id,(empty_chair+1));
			empty_chair = (empty_chair + 1)%NUM_OF_SEATS;
			pthread_mutex_unlock(&mutex_lock);
			sem_post(&students_sem);//Tell TA that a student is waiting for help

			pthread_mutex_unlock(&mutex_lock);

			sem_wait(&ta_sem); //Now wait for TA to help

			//sleep(1);
			help_count++;
			if(help_count==NUM_OF_HELPS) //if already got the maximum number of helps come out
			break;
		}
	}

	printf("\nStudent %d finished and exiting!",id);
	pthread_exit(NULL);
}

int main()
{
	pthread_t ta_thread, student_thread[NUM_OF_STUDENTS];
	int i = 1;
	initialize();

	pthread_create(&ta_thread,NULL,teacher,NULL);

	for(i=1;i<=NUM_OF_STUDENTS;i++)
	{
		pthread_create(&student_thread[i-1],NULL,student, &i);
	}

	//sleep(2);

	for(i=1;i<=NUM_OF_STUDENTS;i++)
	{
		pthread_join(student_thread[i-1],NULL);
	}

	printf("\nAll students finished work. Stopping TA thread...\n");
	pthread_cancel(ta_thread);
}
