#include <stdio.h>
#include "logger.h"

static char dir[255];
static int loggerDirSet = 0;


void setLoggerDirectory(char *directory)
{
    sprintf(dir,"%s",directory);
    loggerDirSet = 1;
}

void writeToLog(char *message,char *filename)
{
    FILE *out;
    char dirAndFilename[255];
    if(loggerDirSet){
        sprintf(dirAndFilename,"%s/%s",dir,filename);
        out = fopen(dirAndFilename,"a+");
    }
    else{
        out = fopen(filename,"a+");
    }

    if(out!=NULL){
        fprintf(out,"%s\n",message);
        fclose(out);
    }
}
void writeSeparatorToLog(char *filename)
{
    FILE *out;
    char dirAndFilename[255];
    if(loggerDirSet){
        sprintf(dirAndFilename,"%s/%s",dir,filename);
        out = fopen(dirAndFilename,"a+");
    }
    else{
        out = fopen(filename,"a+");
    }

    if(out!=NULL){
        fprintf(out,"%s\n","-------------------------------------------------------------------------------------------------------------------------");
        fclose(out);
    }
}


