#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <numeric>

#define MCW MPI_COMM_WORLD

using namespace std;

int gatherOrders(){
    int flag, data;
    int count = 0;
    MPI_Status status;

    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &flag, &status);

    while(flag){
        MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, 0, MCW, &status);
        count++;
        if(count > 20){
            break;
        }
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &flag, &status);
    }
    cout << "got " << count << " orders" << endl;
    return count;
}

void cook(int size){
    //initialize cook
    int data, count, orders = 0;
    int keepWorking = 1;
    int flag;
    MPI_Status status;

    while(keepWorking){
        //check for orders
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &flag, &status);

        //if there is at least one order, gather all of them
        if(flag){
            orders = gatherOrders();
        } else {
            //if no orders, cook smokes a cig
            cout << "Smoking a cig" << endl;
            sleep(1);
        }

        //if more than 20 orders, cook quits
        if(orders > 20){
            cout << "Screw this, the cook has quit" << endl;
            keepWorking = 0;
            data = -1;
                for(int i = 1; i < size; i++){
                MPI_Send(&data, 1, MPI_INT, i, 0, MCW);
            }
            break;
        }

        //go through orders
        while(orders > 0){
            orders--;
            sleep(1);
        }
    }
}

void chef(int rank){
    //initialize chef
    int data, random, count = 0;
    int flag;
    int keepWorking =1;
    srand(time(NULL)*rank*rand());
    MPI_Status status;

    while(keepWorking){
        //make sure the cook hasnt quit
        MPI_Iprobe(0, -1, MCW, &flag, &status);
        if(flag){
            //if cook has quit, chef also quits
            MPI_Recv(&data, 1, MPI_INT, 0, 0, MCW, &status);
            if(data == -1){
                keepWorking = 0;
            }
        }

        //wait between 1 and 5 seconds
        random = (rand()%5) + 1;
        sleep(random);

        //send orders to cook
        // cout << "sent an order" << endl;
        MPI_Send(&data, 1, MPI_INT, 0, 0, MCW);
        count++;
    }
    cout << "The cook has quit, so chef " << rank << " has quit" << endl;
}

int main(int argc, char **argv){
	//set seed for random function
	srand(time(NULL));   

	//initialize MPI
	int rank, size, data;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size);

    //set up chefs and cook
    if(rank == 0){
		cook(size);
	} else {
		chef(rank);
	}

	MPI_Finalize();
    
	return 0;
}