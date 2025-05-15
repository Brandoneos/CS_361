#include "elevator.h"

/*
About this:
starter code without a single semaphore
*/


#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>


static struct passenger {
    int from_floor;
    int to_floor;
} Passenger;

struct pRequest{
    int requestID;
    int passengerId;
    int elevatorId;
    int from_floor;
    int to_floor;
    sem_t passenger_ready_s;
    sem_t elevator_boarding_s;
    sem_t passenger_on_s;
    sem_t elevator_unloading_s;
    sem_t passenger_off_elevator_s;
    struct pRequest* next;
};

typedef struct barrier {
    int waiting;
    int limit;
    sem_t lock;
    sem_t is_open;
} Barrier;
static Barrier B;
static Barrier E;

sem_t lock_for_request_s;
sem_t lock_for_elevator_s;

sem_t last_elevator_s;

int sem_of_elevators;

struct pRequest* headOfRequests;

int passenger_ready_for_pickup;
int passenger_in_elevator;
int passenger_exited_elevator;

int elevator_at_pickup;
int elevator_at_destination;

static void initialize(int blimit,int elimit) {
    B.waiting = 0;
    B.limit = blimit;
    sem_init(&B.lock,0,1);
    sem_init(&B.is_open,0,0);
    E.waiting = 0;
    E.limit = elimit;
    sem_init(&E.lock,0,1);
    sem_init(&E.is_open,0,0);
    sem_of_elevators = 0;

    int semaphore_array[ELEVATORS];
    log(0,"Elevators:%s\n"," ");
    for(int i = 0; i < ELEVATORS; i++) {
        i[semaphore_array] = i;
        log(0,"%d,",i[semaphore_array]);
    }
    log(0,"%s\n"," ");
}   

static void wait(void) {
    sem_wait(&B.lock);
    if(B.waiting < B.limit) {
        B.waiting += 1;
        if(B.waiting == B.limit) {
            sem_post(&B.is_open);
            // sem_post(&lock_for_elevator_s);
            sem_post(&lock_for_elevator_s);
            log(0,"Barrier is open %d\n",0);
        }
    }
    sem_post(&B.lock);
    sem_wait(&B.is_open);
    sem_post(&B.is_open);
}
static void wait1(void) {
    sem_wait(&E.lock);
    if(E.waiting < E.limit) {
        E.waiting += 1;
        if(E.waiting == E.limit) {
            sem_post(&E.is_open);
        }
    }
    sem_post(&E.lock);
    sem_wait(&E.is_open);
    sem_post(&E.is_open);
}


void scheduler_init(void)
{
    memset(&Passenger, 0x00, sizeof(Passenger));
    passenger_ready_for_pickup = 0;
    passenger_in_elevator = 0;
    passenger_exited_elevator = 0;
    elevator_at_pickup = 0;
    elevator_at_destination = 0;
    //added sems
    sem_init(&lock_for_request_s,0,1);
    sem_init(&lock_for_elevator_s,0,0);
    sem_init(&last_elevator_s,0,0);
    log(0,"PASSENGERS:%d\n",PASSENGERS);
    log(0,"ELEVATORS:%d\n",ELEVATORS);
    initialize(PASSENGERS,1);

}



struct pRequest* addRequest(int passenger, int from_floor, int to_floor) {
    //appends the request to the end of the linked list
    struct pRequest* newNode = (struct pRequest*)malloc(sizeof(struct pRequest));
    newNode->from_floor = from_floor;
    newNode->to_floor = to_floor;
    newNode->passengerId = passenger;
    newNode->next = NULL;
    newNode->elevatorId = -1;

    sem_init(&(newNode->passenger_ready_s),0,0);
    sem_init(&(newNode->elevator_boarding_s),0,0);
    sem_init(&(newNode->passenger_on_s),0,0);
    sem_init(&(newNode->elevator_unloading_s),0,0);
    sem_init(&(newNode->passenger_off_elevator_s),0,0);
    

    if(headOfRequests == NULL) {
        newNode->requestID = 0;
        headOfRequests = newNode;
    } else {
        struct pRequest* pointer = headOfRequests;
        int id = 0;
        while(pointer->next != NULL) {
            pointer = pointer->next;
            id++;
        }
        id++;
        newNode->requestID = id;
        pointer->next = newNode;

    }
    return newNode;

}
bool matchesRequest(struct pRequest* req, int passenger, int from_floor, int to_floor) {
    if(req == NULL) {
        return false;
    }
    if(req->from_floor == from_floor && req->passengerId == passenger && req->to_floor == to_floor) {
        return true;
    }
    return false;
}

void removeRequest(int passenger, int from_floor, int to_floor) {
    struct pRequest* pointer = headOfRequests;
    if(headOfRequests == NULL) {
        return;
        /*
        else if(headOfRequests->next == NULL) {
        if(matchesRequest(pointer,passenger,from_floor,to_floor)) {
            struct pRequest* nodeToRemove = headOfRequests;
            headOfRequests = headOfRequests->next;
            nodeToRemove->next = NULL;
            free(nodeToRemove);
            return;
        }
        */
    } else if(matchesRequest(pointer,passenger,from_floor,to_floor)) {
        //need to remove head
        struct pRequest* nodeToRemove = headOfRequests;
        headOfRequests = headOfRequests->next;
        nodeToRemove->next = NULL;
        free(nodeToRemove);
        return;
    }
    struct pRequest* previous = headOfRequests;
    while(pointer->next != NULL) {
        pointer = pointer->next;
        if(matchesRequest(pointer,passenger,from_floor,to_floor)) {
            struct pRequest* nodeToRemove = pointer;
            previous->next = nodeToRemove->next;
            free(nodeToRemove);
            break;
        }
        previous = previous->next;
    }
    return;
}




void passenger_request(int passenger, int from_floor, int to_floor,
                       void (*enter)(int, int), void (*exit)(int, int))
{
    // log(0,"test : %d\n",passenger);
    // log(0,"Passenger_request() called from %d\n",passenger);
    //controller for passenger
    
    // inform elevator of floor
    sem_wait(&lock_for_request_s);
    struct pRequest* currentRequest = addRequest(passenger,from_floor,to_floor); 
    sem_post(&lock_for_request_s);

    wait();//barrier for passenger threads
    // sem_of_elevators = 1;
    
    // Passenger.from_floor = from_floor;
    // Passenger.to_floor = to_floor;


    // signal ready and wait
    passenger_ready_for_pickup = 1;
    // log(0,"posting passenger %d with request: id:%d,from:%d,to:%d\n",passenger,passenger,from_floor,to_floor);
    sem_post(&(currentRequest->passenger_ready_s));//(1)
    // int value;
    // sem_getvalue(&currentRequest->passenger_ready_s,&value);//sem value is 0
    // log(0,"passenger %d with request's prs:%d\n",passenger,value);
    // while (!elevator_at_pickup);
    sem_wait(&(currentRequest->elevator_boarding_s));//(2)
    // while(!elev);
    // struct pRequest* currentRequest = headOfRequests;
    // while(currentRequest->next != NULL) {
    //     if(matchesRequest(currentRequest,passenger,from_floor,to_floor)) {
    //         break;
    //     }
    //     currentRequest = currentRequest->next;
    // }
    while(currentRequest->elevatorId == -1) {
    }
    int elevatorid = currentRequest->elevatorId;
    // log(0,"passenger %d got elevator id %d\n",passenger,elevatorid);
    // enter elevator and wait
    enter(passenger, elevatorid);//api call, passengerid:0, elevatorid:0
    // log(0,"passenger %d at elevator id %d entered the elevator\n",passenger,elevatorid);
    passenger_in_elevator = 1;
    sem_post(&(currentRequest->passenger_on_s));//(3)
    // while (!elevator_at_destination);

    // exit elevator and signal
    sem_wait(&(currentRequest->elevator_unloading_s));//(4)
    exit(passenger, elevatorid);//api call, passengerid:0, elevatorid:0
    passenger_exited_elevator = 1;
    passenger_ready_for_pickup = 0;
    sem_post(&(currentRequest->passenger_off_elevator_s));//(5)
    // log(0,"Passenger_request() returned from %d\n",passenger);
}


// example procedure that makes it easier to work with the API
// move elevator from source floor to destination floor
// you will probably have to modify this in the future ...

static void move_elevator(int elevator,int source, int destination, void (*move_direction)(int, int))
{
    
    int direction, steps;
    int distance = destination - source;
    if (distance > 0) {
        direction = 1;
        steps = distance;
    } else {
        direction = -1;
        steps = -1*distance;
    }
    for (; steps > 0; steps--)
        move_direction(elevator, direction);
}


void elevator_ready(int elevator, int at_floor,
                    void (*move_direction)(int, int), void (*door_open)(int),
                    void (*door_close)(int))
{
    
    sem_wait(&lock_for_elevator_s);
    if(headOfRequests->next == NULL && headOfRequests->elevatorId != -1) {
        // log(0,"last request is taken from elevator %d:\n",elevator);
        // pthread_t self = pthread_self();
        // pthread_detach(self);
        // log(0,"elevator_ready returns1 (%ld)\n",self);
        //waits for last request to be removed then returns 
        sem_post(&lock_for_elevator_s);
        sem_wait(&last_elevator_s);
        sem_post(&last_elevator_s);
        log(0," elevator returns from middle %d:\n",elevator);
        //after the elevator which completed that last request
        return;
        // pthread_exit(0);
    } 
    log(0,"Grabbing request at elevator %d:\n",elevator);
    // wait for passenger to press button and move
    // while (!passenger_ready_for_pickup);
    // pthread_t self = pthread_self();
    struct pRequest* currentRequest = headOfRequests;
    // int c1 = currentRequest.
    // log(0,"Grabbing request at elevator %d:\n",elevator);
    while(currentRequest != NULL) {
        if(currentRequest->elevatorId == -1) {
            break;
        }
        currentRequest = currentRequest->next;
    }
    if(currentRequest == NULL) {
        log(0," elevator %d is waiting at middle2 :\n",elevator);
        sem_post(&lock_for_elevator_s);
        sem_wait(&last_elevator_s);
        sem_post(&last_elevator_s);
        log(0," elevator returns from middle2  %d:\n",elevator);
        return;//for now when elevators exceed passengers
    }
    log(0,"Grabbed request at elevator %d:\n",elevator);
    currentRequest->elevatorId = elevator;
    sem_post(&lock_for_elevator_s);

    // int value;
    // sem_getvalue(&currentRequest->passenger_ready_s,&value);//sem value is 0
    // log(0,"elevator %d with request's prs:%d,id:%d,from:%d,to:%d\n",elevator,value,currentRequest->passengerId,currentRequest->from_floor,currentRequest->to_floor);
    sem_wait(&(currentRequest->passenger_ready_s));//(1)
    // log(0,"elevator_ready() called from self %d\n",elevator);
    // log(0,"List of current requests at elevator %d:\n",elevator);

    // struct pRequest* cur = headOfRequests;
    // while(cur != NULL) {
    //     log(0,"Request at: %d\n",cur->requestID);
    //     cur = cur->next;
    // } = 0-19 requests

    
    
    
    // log(0,"reserved spot at elevator %d, passenger: %d, from: %d, to:%d\n",elevator,currentRequest->passengerId,currentRequest->from_floor,currentRequest->to_floor);
    // sem_post(&lock_for_elevator_s);
    // int p_from_floor 

    //just reading from shared data structure 
    // log(0,"moving elevator %d:\n",elevator);
    move_elevator(elevator,at_floor, currentRequest->from_floor, move_direction);

    // open door and signal passenger
    door_open(elevator);
    elevator_at_pickup = 1;
    // currentRequest->elevatorId = elevator;
    sem_post(&(currentRequest->elevator_boarding_s));//(2)

    // wait for passenger to enter then close and move
    // while (!passenger_in_elevator);
    sem_wait(&(currentRequest->passenger_on_s));//(3)
    // log(0,"passenger:%d got on elevator: %d\n",currentRequest->passengerId,elevator);
    door_close(elevator);
    move_elevator(elevator,currentRequest->from_floor,currentRequest->to_floor,move_direction);

    // open door the signal
    door_open(elevator);
    elevator_at_destination = 1;
    sem_post(&(currentRequest->elevator_unloading_s));//(4)

    // wait for passenger to leave and close door
    // while (!passenger_exited_elevator);
    sem_wait(&(currentRequest->passenger_off_elevator_s));//(5)
    door_close(elevator);
    
    elevator_at_pickup = 0;
    sem_wait(&lock_for_elevator_s);//are these needed?, don't seem to make difference
    removeRequest(currentRequest->passengerId,currentRequest->from_floor,currentRequest->to_floor);
    sem_post(&lock_for_elevator_s);
    log(0,"elevator removed request(%d)\n",elevator);
    //head is null after all is done
    pthread_t self = pthread_self();
        // pthread_detach(self);
    
    if(headOfRequests == NULL) {
        log(0,"elevator_ready returns in the end(%ld)(%d)\n",self,elevator);
        sem_post(&last_elevator_s);
    }
    
}
