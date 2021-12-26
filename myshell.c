#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include<sys/stat.h>
#include <dirent.h>

size_t size;
//char* ptr;

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

void throwError()
{
    char error_message[30] = "An error has occurred\n";
    write(STDOUT_FILENO, error_message, strlen(error_message));
}


//check for whitespace??
void pwd()
{
    char dir[PATH_MAX];
    if (getcwd(dir, sizeof(dir)) != NULL) 
    {
       myPrint(dir);
       myPrint("\n");
    } 
    else 
    {
       throwError();
   }
}


void cd(char* dest)
{
    int success;
    success = chdir(dest);
    if (success !=0)
    {
        throwError();
    }
}



int isEmpty(char * instr)
{
    char str[strlen(instr)];
    strcpy(str, instr);
    if (strtok(str, "\t \n") == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int fileExists(char* name)
{
    if( access( name, F_OK ) == 0 ) 
    {
        return 1;
    } 
    else 
    {
        return 0;
    }
}

int hasRedirection(char** arr, int length)
{
    for (int i = 0; i< length-1; i++)
    {
        if (strcmp(arr[i], ">") ==0)
        {
            //myPrint("red ");
            return 1;
        }
    }
    return 0;
}

int correctRedirectionPos(char** arr, int length)
{
    if (length < 3)
    {
        return 0;
    }

    if ((strcmp( arr[length-3], ">") ==0) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int redCnt(char** arr, int length)
{
    int cnt=0;
    for (int i = 0; i< length-1; i++)
    {
        if (strcmp(arr[i], ">") ==0)
        {
            cnt++;
        }
    }
    return cnt;
}


int correctAdvRed( char** arr, int length)
{
    if (length < 4)
    {
        return 0;
    }
    if ((strcmp( arr[length-4], ">") ==0)  && (strcmp( arr[length-3], "+") ==0) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char* addSpaces(char* str)
{
    //char newstr[strlen(str)+3];
    char* newstr = (char*) malloc((strlen(str)+3)* sizeof(char));
    int pos=0;
    for (int i=0; i< strlen(str); i++)
    { 
        if (str[i] == '>')
        {
            pos = i;
        }
    }
    if (pos ==0)
    {
        return str;
    }
    for (int i = 0; i<pos ; i++)
    {
        newstr[i]= str[i];
    }
    newstr[pos] = ' ';
    newstr[pos+1] = '>';
    newstr[pos+2] = ' ';
    for (int i = pos+1; i < strlen(str); i++)
    {
        newstr[i+2]= str[i];
    }
    newstr[strlen(str)+2] = '\0';

    return newstr;
}

char* addSpacesPlus(char* str)
{
    //char newstr[strlen(str)+3];
    char* newstr = (char*) malloc((strlen(str)+3)* sizeof(char));
    int pos=0;
    for (int i=0; i< strlen(str); i++)
    { 
        if (str[i] == '+')
        {
            pos = i;
        }
    }
    if (pos ==0)
    {
        return str;
    }
    for (int i = 0; i<pos ; i++)
    {
        newstr[i]= str[i];
    }
    newstr[pos] = ' ';
    newstr[pos+1] = '+';
    newstr[pos+2] = ' ';
    for (int i = pos+1; i < strlen(str); i++)
    {
        newstr[i+2]= str[i];
    }
    newstr[strlen(str)+2] = '\0';

    return newstr;
}



void executeInstruction(char* instr)
{
    char* cmdArr[514];

    char instrcpy[strlen(instr)];
    strcpy(instrcpy, instr);


    int redirection =0;    
    int advanced = 0;
    int fd;
    char* ptr;
    int dne = 0;

    FILE* srce;
    FILE* dest;
    char ch;

    char* newinstr = addSpacesPlus(addSpaces(instrcpy));

    char* piece = strtok_r(newinstr, "\t \n", &ptr); //removew new
    //char* piece = strtok(newinstr, "\t \n");

    if (piece == NULL)
    {
        return;
    }
    cmdArr[0] = piece;
    int count = 1;
    pid_t forkret =0; 
    while (piece != NULL)
    {
       piece = strtok_r(NULL, "\t \n", &ptr);
       //piece = strtok(NULL, "\t \n");
       cmdArr[count] = piece;
       count++;
    }
    

    //fine
    if (strcmp(cmdArr[0], "exit") ==0)
    {
        if (cmdArr[1] == NULL)
        {
            exit(0);
        }
        else
        {
            throwError();
        }
    }
    else if(strcmp(cmdArr[0], "cd")==0)
    {
        if (cmdArr[2] == NULL && cmdArr[1] !=NULL)
        {
            cd(cmdArr[1]);
        }
        else if (cmdArr[1]==NULL)
         {
            chdir(getenv("HOME"));
         }
         else
         {
            throwError();
         }
    }
    else if(strcmp(cmdArr[0], "pwd")==0)
    {
        if (cmdArr[1] == NULL)
        {
            pwd();
        }
        else
        {
            throwError();
        }
    }
    //fine


    else  //not built in
    {


        if (hasRedirection(cmdArr, count))
        {
        
            if ((!correctRedirectionPos(cmdArr, count) && !correctAdvRed(cmdArr, count) )|| (redCnt(cmdArr, count) != 1) ) //count <= 1?
            {
                    throwError();
                    return;
            }   

            if (correctRedirectionPos(cmdArr, count))
            {
                redirection =1;  
            }
            else if(correctAdvRed(cmdArr, count))
            {
                advanced = 1;
            }


            else
            {
                throwError();
                return;
            }
              
        }

        char* cmd;
        char* args[count-2*redirection - 3*advanced];
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        if ((redirection==0 )&& (advanced ==0))
        {
            for (int i = 0 ; i< count; i++)
            {
                args[i]= cmdArr[i];
            }
            cmd = args[0];
        }
        else if ((redirection ==1) && (advanced ==0))
        {
            for(int i =0; i< count-3; i++)
            {
                args[i]= cmdArr[i];
            }
            args[count-3] = NULL;
            cmd = args[0];
        }
        else if (redirection ==0 && advanced ==1)
        {
            for(int i =0; i< count-4; i++)
            {
                args[i]= cmdArr[i];
            }
            args[count-4] = NULL;
            cmd = args[0];
        }


        if ((forkret = fork())==0)//child
        {
            //if > (correct syntax) dup2 ()
            if (redirection)
            {
                if (fileExists(cmdArr[count-2]))
                {
                    throwError();
                    exit(0);
                }

                fd = creat(cmdArr[count-2], mode);

                if (fd <0)
                {
                    throwError();
                    exit(0);
                }
                dup2(fd,1);
            }
            else if (advanced)
            {             
                if (!fileExists(cmdArr[count-2]))
                {
                    fd = creat(cmdArr[count-2], mode);
                    if (fd <0)
                    {
                        throwError();
                        exit(0);
                    }
                    dup2(fd,1);
                }
                else
                {
                    int fd2 = open(cmdArr[count-2], O_RDWR , mode );
                    if (fd2 <0)
                    {
                        //dne =1;
                        throwError();
                        exit(0);
                    }
                    close(fd2);
                    //write to beginning of file
                    fd = creat("newf", mode);
                    dup2(fd, 1);
                }
            }

            int ret = execvp(cmd, args);
            if (ret ==-1)
            {
                throwError();
            }

            exit(0);
        }
        else //parent
        {
            wait(NULL);

            if (access(cmdArr[count-2], F_OK)!= 0)
            {
                return;
            }
            if (advanced==1 && dne==0)
            {         
                srce = fopen(cmdArr[count-2], "r");
                dest = fopen("newf", "a");
                ch = fgetc (srce);//problem
                while (ch != EOF)
                {
                    fputc(ch, dest);
                    ch = fgetc (srce);
                }
                fclose(srce);
                fclose(dest);
                remove(cmdArr[count-2]);
                rename("newf", cmdArr[count-2]);        
 


            }



        }
    }
    
}




int main(int argc, char *argv[]) 
{
    char cmd_buff[514];
    char *pinput;
    char* curr; 
    size_t size;
    size_t ret;
    FILE* batchFile;
    char* ptr;

    //size_t lineSize ; //= getline(&pinput, &size, stdin);
    if (argc < 2)
    {
        while (1) {
            myPrint("myshell> ");
            pinput = fgets(cmd_buff, 514, stdin);

            if (pinput==NULL) //couldn't read input, should be null???
            {
                throwError();
                exit(0);
            }
            else
            {
                curr= strtok_r(pinput, ";", &ptr);
                while (curr!= NULL)
                {
                    executeInstruction(curr);
                    curr = strtok_r(NULL, ";", &ptr);
                }
            }
        }
    }
    //batch mode
    else if (argc ==2)
    {
        batchFile = fopen(argv[1], "r");
        if (batchFile == NULL)
        {
            throwError();
            exit(0); 
        }
        ret = getline( &pinput, &size, batchFile); 
        while (ret != EOF)
        {
            if (!isEmpty(pinput))
            {
                myPrint(pinput); //problem
                
            }

            if (strlen(pinput) > 512)
            {
                throwError();
                //continue;
            }

            curr= strtok_r(pinput, ";", &ptr);
                while (curr!= NULL)
                {
                    executeInstruction(curr);
                    curr = strtok_r(NULL, ";", &ptr);
                    
                }
            ret = getline( &pinput, &size, batchFile); 
        
        }
        
    }
    else
    {
        throwError();
    }

}
