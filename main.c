#include <stdio.h>
#include <string.h>
#include "mpi.h"

const int MX = 1000;
int main(int argc, char* argv[]) {
	int rank, siz , each;
	int keyShift;
	char msg[MX] ;
	char rcvBfr[MX];
	char sendBfr[MX];
	char res[MX] ;
	FILE* fPtr;
	/// initialize recv buffer with null string terminator
	int i ;
	for ( i=0 ; i<MX ; i++) {
		rcvBfr[i] = '\0';
	}
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &siz);

	if (rank == 0) {
		char str[MX];
		// read the name of the file
		printf("Enter The Name Of The File\n");
		fflush(stdout);
		gets(str);
		char ext[] = ".txt";
		int flag = 1;		// flag = 0 , if the file name don't end with the extenstion
		// check if user entered the name with or without the extention
		int len = strlen(str);
		int j ;
		for (i = len - 4 , j=0 ; i>=0 && i<len ; i++ , j++){
			if (str[i] != ext[j])
				flag = 0;
		}
		if (!flag)	// if flag = false , the user entered the name without the extention
			strcat(str, ext);
		// read the key value
		printf("Enter The Key Shift\n");
		fflush(stdout);
		scanf("%d", &keyShift);

		// read the msg from the given file

		fPtr = fopen(str,"r"),
		fgets(msg, MX, fPtr);
		fclose(fPtr) ;
		// append white spaces on the msg untill the size is divisible by the number of processes
		int slaves = siz - 1;
		while (strlen(msg) % slaves != 0) {
			strcat(msg, " ");
		}

		// calc number of chars to be sent to each process
		each = strlen(msg) / (slaves);
		for (i = 0; i<each ; i++) {
				// send dummy data to master process
				sendBfr[i] = 'A';
		}
		int msgInd = 0 , ind=each;
		for (i = 0; i < slaves ; i++){
			int eachTemp = each;
			while (eachTemp--) {
				sendBfr[ind++] = msg[msgInd++];
			}
		}
	}
	// podcast the key value to all slaves
	MPI_Bcast(&keyShift, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&each, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// scatter the msg on
	MPI_Scatter(&sendBfr, each , MPI_CHAR, &rcvBfr, each , MPI_CHAR,0, MPI_COMM_WORLD);

	// encrypt the msg
	if(rank!=0){
		for(i=0 ; i<strlen(rcvBfr) ; i++){
			char newC = rcvBfr[i];
            if(rcvBfr[i]>='a' && rcvBfr[i]<='z') {
                newC = (char) ((rcvBfr[i]-'a' + keyShift) % 26 + 'a');
            }
            else if(rcvBfr[i]>='A' && rcvBfr[i]<='Z') {
                newC = (char) ((rcvBfr[i]-'A' + keyShift) % 26 + 'A');
			}
			rcvBfr[i] = newC;
		}
	}

	MPI_Gather(&rcvBfr,each,MPI_CHAR,&res,each,MPI_CHAR,0,MPI_COMM_WORLD) ;

	if(rank==0){
		fPtr = fopen("output.txt","w");
		for(i=each ; i<strlen(res) ; i++)
			fprintf(fPtr,"%c",res[i]) ;
		fclose(fPtr) ;
	}

	MPI_Finalize();
	return 0;
}
