//Chen Sabban - 205983836
//Dan Ben Natan - 313196966

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include <string.h>
#include<sys/wait.h> 



int addLineToCSV(char* folderName, int boolean);

int main(int argc, char* argv[]) {
	int fdConfig;
	DIR* PointerStudents;
	struct dirent* direntStudents;

	// Check arguments number
	if(argc != 2) { 
		printf("wrong arguments \n");
		exit(-1);
	}

	fdConfig = open(argv[1], O_RDONLY);
	if (fdConfig < 0) { //*means file open did not take place
		printf("Problem with opening the config file \n"); 
		perror("Error after open");
		exit(-1);
	}

	//Reads the lines from the fd:
	char line1[50];
	char line2[50];
	char line3[50];
	int currentLine = 1;
	int i = 0;
	char buf;
	int fdInput, fdExpectedOutput;
	while (read(fdConfig, &buf, 1) > 0) {
		if (buf != '\n') {
			switch (currentLine) {
			case 1:
				line1[i] = buf;
				i++;
				break;
			case 2:
				line2[i] = buf;
				i++;
				break;
			case 3:
				line3[i] = buf;
				i++;
				break;
			}

		}
		else if (buf == '\n') {
			currentLine++;
			i = 0;
		}
	}

	//Open students folder
	// Opens the directory from the 1st line in the configuration file
	printf("line1: %s\n", line1);
	PointerStudents = opendir(line1);   //gets a pointer where the students name folder
	if (PointerStudents == NULL) { //*means dir open did not take place
		perror("Error after open the directory from the 1st line in the configuration file");
		close(fdConfig);
		exit(-1);
	} 
	

	// parameters of the while
	DIR* pStudent;
	int status;
	struct dirent* direntStud;

	while ((direntStudents = readdir(PointerStudents)) != NULL) {  //going through students dir 

		if (strcmp(direntStudents->d_name, ".") == 0 || strcmp(direntStudents->d_name, "..") == 0)
			continue;

		int keyBoard = dup(0), screen = dup(1);

		// Build file and destnation paths

		// studentDir: path of the dir of the specific student    
		//  students/chen
		int sizeName = strlen(line1) + strlen(direntStudents->d_name)+1; // gets the length of the student dir name 
		char studentDir[sizeName];  
		strcpy(studentDir, line1);
		strcat(studentDir, direntStudents->d_name);
		
		// pathStudentC: path of the C file of the specific student
		//  students/chen/chen.c
		int sizeC = strlen(studentDir) + strlen(direntStudents->d_name) +4;
		char pathStudentC[sizeC];   // path of the C file of the specific student 
		strcpy(pathStudentC, studentDir);
		strcat(pathStudentC, "/");
		strcat(pathStudentC, direntStudents->d_name);
		strcat(pathStudentC, ".c");

		// destPathStudent: path of the main.out file of the specific student
		//  students/chen/main.out
		int dest = strlen(studentDir) + 10; // +  /main.out
		char destPathStudent[dest];
		strcpy(destPathStudent, studentDir);
		strcat(destPathStudent, "/main.out");

		//Run gcc command
		pid_t ppid ; // gets the current process id 
		int returned_value;

		char* gcc_arguments[]= { "gcc" , pathStudentC ,  "-o" , destPathStudent , NULL }; //Build gcc command
		ppid = fork();
		if (ppid == 0) {   //  // I am the son
			execvp(gcc_arguments[0], gcc_arguments);
			// If we got this far, it means something went wrong,
			// In that case, we'll simply declare this folder as zero score.
			exit(-1); // We won't need two processes to proceed.
		}
		
		//I am the father
		wait(&returned_value);
		
		//check if the student file succeed to compile
		if (WEXITSTATUS(returned_value) < 0 ) {
			// Append new line of the sub-folder (de->d_name) with score 0 to the CSV.
			int check = addLineToCSV(direntStudents->d_name, 1);
			if(check < 0 ){  //check if the function succeed to write to csv 
				printf("Problem with writing to the csv file \n");
				exit(-1);
			}
			break; // Assuming that there's only one CORRECT file ( .c file )
		}


		// open input.qa , expected_output.txt
		fdInput = open(line2, O_RDONLY);
		if (fdInput < 0) { //*means file open did not take place
			perror("Error after open input.qa");
			closedir(pStudent);
			exit(-1);
		}
		
		// studentDir: path of the dir of the specific student    
		//  students/chen/program_output.txt
		int size = strlen(studentDir) + 19;
		char studentProgramOutput[size];  
		strcpy(studentProgramOutput, studentDir);
		strcat(studentProgramOutput, "/");
		strcat(studentProgramOutput, "program_output.txt");

		int fdPOutput = open(studentProgramOutput, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (fdPOutput < 0) { //*means file open did not take place
			printf("error open progOutput");
			closedir(pStudent);
			close(fdInput);
			exit(-1);
		}


		//Running each student program   ./ command on main.out
		char command[50]="";
		strcpy(command, "./Students/");
		strcat(command, direntStudents->d_name);
		strcat(command, "/main.out");

		//Read from input file and write to program_output file
		dup2(fdInput, 0); 
		dup2(fdPOutput, 1);

		char* runMain_arguments[]= { command , NULL };
		ppid = fork();
		if (ppid == 0) {   //  // I am the son
			execvp(runMain_arguments[0], runMain_arguments);
			// If we got this far, it means something went wrong,
			// In that case, we'll simply declare this folder as zero score.
			exit(-1); // We won't need two processes to proceed.
		}
		
		//I am the father
		wait(&returned_value);
		
		//check if the student file succeed to run
		if (WEXITSTATUS(returned_value) <0 ) {
			// Append new line of the sub-folder (de->d_name) with score 0 to the CSV.
			int check = addLineToCSV( direntStudents->d_name, 1);
			if(check < 0 ){  //check if the function succeed to write to csv 
				printf("Problem with writing to the csv file \n");
				exit(-1);
			}
			//Return to defult
			dup2(keyBoard, 0);
			dup2(screen, 1);
			break; // Assuming that there's only one CORRECT file ( .c file )
		}

		//Return to defult
		dup2(keyBoard, 0);
		dup2(screen, 1);

		//open the expected output file
		fdExpectedOutput = open(line3, O_RDONLY);
		if (fdExpectedOutput < 0) { //*means file open did not take place
			printf("Problem with opening the excpected output \n");
			perror("Error after open");
			closedir(pStudent);
			close(fdInput);
			close(fdPOutput);
			exit(-1);
		}

		// run the compare process
		char* runComp_arguments[] = { "./comp.out",studentProgramOutput, line3 , NULL };
		ppid = fork();
		if (ppid == 0) {   //  // I am the son
			execvp(runComp_arguments[0], runComp_arguments);
			// If we got this far, it means something went wrong,
			// In that case, we'll simply declare this folder as zero score.
			exit(-1); // We won't need two processes to proceed.
		}
		//I am the father
		wait(&returned_value);
		
		if (WEXITSTATUS(returned_value) <0 ) {
			printf("Problem with comparing the files \n");
			closedir(pStudent); close(fdPOutput);
			close(fdInput); close(fdExpectedOutput); return -1;
		}
		
		//checking if files are match, and update result CSV
		if (WEXITSTATUS(returned_value) == 2)
			addLineToCSV( direntStudents->d_name, 2);
			
		else if (WEXITSTATUS(returned_value) == 1)
			addLineToCSV(direntStudents->d_name, 1);
	

		close(fdPOutput);
		closedir(pStudent);
		close(fdInput);
		close(fdExpectedOutput);
	}	
	
	
	//Print csv file
	int statusp;
	printf("\n\n----------------- RESULT.CSV -----------------\n\n");
	char* cat_arguments[] = { "cat" , "result.csv" , NULL };
	if (fork() == 0) {
			execvp(cat_arguments[0], cat_arguments);
			exit(-1);
	}
	wait(&statusp);
	printf("\n------------------- DONE ---------------------\n\n\n\n");

	close(fdConfig);
	close(fdInput);
	close(fdExpectedOutput);
	closedir(PointerStudents);

	return 0;

}

int addLineToCSV(char* folderName, int boolean)
{
	int fdCsv = open("result.csv", O_WRONLY | O_APPEND);
	if (fdCsv == -1)
	{
		printf("Problem with opening the csv file \n");
		exit(-1);
	}

	int len = strlen(folderName) +3 ;
	char buff[len];
	strcpy(buff, folderName);
	if (boolean == 2){
		strcat(buff,",");
		strcat(buff,"2");
		strcat(buff,"\n");
	}	
	else if (boolean == 1){
		strcat(buff,",");
		strcat(buff,"1");
		strcat(buff,"\n");
	}
	
	if (write(fdCsv, buff, len) <0) {
		close(fdCsv);
		return -1;
	} 
	close(fdCsv);
	return 0;
}