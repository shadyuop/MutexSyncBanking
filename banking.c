#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct Transaction {
  int ID;
  double timeRecieved;
  int idSend;
  int idRecieve;
  int amount;
  int timeNeeded;
  // int test;
};

struct Bank {
  int id;
  int balance;
};

// Declarations
int toInt(char a[]);
void* spawningThreads(void *arg);
void* bankRequest(int ID, int idSend, int idRecieve, int amount, int timeNeeded);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Global Variables
int accCount = 0;
struct Bank bank[100];


int main()
{

    // Reading Bank Data **************************************************************
    char line[250];
    FILE *fp2 = fopen("balance.txt", "r");

    while (fgets(line, sizeof(line), fp2) != NULL){

        char *ptr = strtok(line, " ");
        bank[accCount].id = toInt(ptr);

        ptr = strtok(NULL, ",");
        bank[accCount].balance = toInt(ptr);

        accCount++;
        printf("Account Read %d\n", accCount);
    }
    fclose(fp2);

    // Reading Transactions Data  *******************************************************
    char line2[250];
    struct Transaction transactions[100];
    FILE *fp = fopen("data.txt", "r");

    int transCount = 0;
    while (fgets(line2, sizeof(line2), fp) != NULL){
        transactions[transCount].ID = transCount +1 ;

        char *ptr = strtok(line2, " ");
        transactions[transCount].timeRecieved = atof(ptr);

        ptr = strtok(NULL, " ");
        transactions[transCount].idSend = toInt(ptr);

        ptr = strtok(NULL, " ");
        transactions[transCount].idRecieve = toInt(ptr);

        ptr = strtok(NULL, " ");
        transactions[transCount].amount = toInt(ptr);

        ptr = strtok(NULL, ",");
        transactions[transCount].timeNeeded = toInt(ptr);

        transCount++;
        printf("Transactions: %d \n", transCount );

    }
    fclose(fp);

    // Spawning TCP Requests TimeSimulated
    printf("\nTransactions Total: %d \n", transCount );
    pthread_t threadRequest[transCount];
    for (int i=0; i<transCount; i++) {
      pthread_create(&threadRequest[i], NULL, spawningThreads, &transactions[i]);
    }

    // Waiting for threads to finish
    for (int i = 0; i < transCount; i++)
    {
      pthread_join(threadRequest[i], NULL);
      printf("End thread %d\n", i);  
    }

    //output Terminal
    printf("\n\n\n\n\nThe Final blance\n");
    printf("*********************\n");
    printf("ID        Balance\n");
    for (int i = 0; i < accCount; i++) {
      printf("%d   %d\n",bank[i].id ,bank[i].balance);
    }

    //output file
    FILE *fpOut;
    fpOut = fopen ("blanceOut.txt","w");
 
   /* write 10 lines of text into the file stream*/
   for(int i = 0; i < accCount;i++){
       fprintf (fpOut, "%d %d,\n",bank[i].id ,bank[i].balance);
   }
 
   /* close the file*/  
   fclose (fp);


    return 0;
}

int toInt(char a[]) {
  int c, sign, offset, n;

  if (a[0] == '-') {  // Handle negative integers
    sign = -1;
  }

  if (sign == -1) {  // Set starting position to convert
    offset = 1;
  }
  else {
    offset = 0;
  }

  n = 0;

  for (c = offset; a[c] != '\0'; c++) {
    n = n * 10 + a[c] - '0';
  }

  if (sign == -1) {
    n = -n;
  }

  return n;
}

void* spawningThreads(void *arg) {
      struct Transaction *transPtr = 
        (struct Transaction*) arg;

      sleep(transPtr->timeRecieved);
      printf("Data recieved from : %d\n", transPtr->idSend);

      bankRequest(transPtr->ID, transPtr->idSend, transPtr->idRecieve, transPtr->amount, transPtr->timeNeeded);
      pthread_exit(0);
}

void* bankRequest(int ID, int idSend, int idRecieve, int amount, int timeNeeded) {

  pthread_mutex_lock(&mutex);
  printf("SenderID Trying to  Access: %d Transaction ID: %d\n", idSend, ID);
  // checking whether is the Sender id Exists
  for (int i=0; i < accCount;i++) {

    if (bank[i].id == idSend) {
      printf("Accessed Successfully Transaction: %d\n", ID);

      //Checking Balance
      if (bank[i].balance < amount){
        printf("Low Balance .. Failed Transaction %d\n\n\n", ID);
        pthread_mutex_unlock(&mutex);
        pthread_exit(0);

        //Balance available
      } else {
        //Check Reaciver
        for (int j=0; j< accCount; j++){

          if (bank[j].id == idRecieve) {
            printf("Found reciever %d of Transaction: %d\n",idRecieve, ID );
            printf("timeNeeded for transaction: %d %d Seconds\n\n\n",ID ,timeNeeded);
            sleep(timeNeeded);
            bank[j].balance += amount;
            bank[i].balance -= amount;
            printf("Transaction %d Success!\n\n", ID);
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
          } 
        }
        //If Reciever Doesnot Exist
        printf("Reciever %d Not Found of Transaction: %d\n\n\n", idRecieve, ID);
        pthread_mutex_unlock(&mutex);
        pthread_exit(0);
      }
    }
  } 
  //if Sender not found
  printf("Sender %d not found of Transaction: %d\n\n\n",idSend, ID );
  pthread_mutex_unlock(&mutex);
}

