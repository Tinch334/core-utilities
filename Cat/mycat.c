#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <math.h>

/*
TODO:


DONE:
-Implement file printing
-Add line numbering option -n
-Add show ends option -e
-Multiple file printing
-Add filename display option
-Add help option
*/


struct flagsStruct {
    int showLinesFlag;
    int showEndsFlag;
    int showFilenameFlag;
};


void displayFile(const char *filename, struct flagsStruct programFlags);
int countLines(const char *filename);


int main(int argc, char **argv)
{
    //Argument parsing
    char c;
    struct stat fileStats;
    //We use a struct to store argument flags to avoid having multiple variables.
    struct flagsStruct programFlags;

    while ((c = getopt (argc, argv, "hnef")) != -1) {
        switch (c) {
            case 'h':
                fprintf(stderr, "Usage: mycat [OPTION/S] [FILE/S]");
                fprintf(stderr, "\nConcatenates FILE/S to standard output.");
                fprintf(stderr, "\n\n  -n\tNumbers each line\n  -e\tShows a '$' at the end of each line\n  -f\tShows each file/s filename before printing it\n  -h\tDisplay this help message\n");
                fprintf(stderr, "\nVersion: 1.0 - By: Martin Goni - 2022\n");
                
                return 1;
            case 'n':
                programFlags.showLinesFlag = 1;
                break;
            case 'e':
                programFlags.showEndsFlag = 1;
                break;
            case 'f':
                programFlags.showFilenameFlag = 1;
                break;
            case '?':
                fprintf (stderr, "Unknown option `-%c', use '-h' for help.\n", optopt);

                //If we got an invalid option we end the program.
                return 1;
        }
    }

    //We check that at least one file was entered for output.
    if (optind >= argc) {
        fprintf(stderr, "Please enter at least one file to output, use '-h' for help.\n");

        //If we got an invalid option we end the program.
        return 1;
    }

    //We print all the arguments supplied after the arguments.
    while (optind < argc) {
        if (stat(argv[optind], &fileStats) == 0) {
            //Check that we have read permission on the file.
            if (!fileStats.st_mode & R_OK) {
                //We flush stdout to force anything left in the standard output to be printed before we print the error.
                fflush(stdout);
                fprintf(stderr, "\n\nYou don't have permission to read '%s', use '-h' for help.\n", argv[optind]);
                return 1;
            }

            //In case the given file is not a regular file.
            if (!S_ISREG(fileStats.st_mode & S_IFMT)) {
                //We flush stdout to force anything left in the standard output to be printed before we print the error.
                fflush(stdout);
                //We add vertical padding.
                fprintf(stderr, "\n\n");

                //Show the correct error depending on the type of the file.
                switch(fileStats.st_mode & S_IFMT) {
                    case S_IFSOCK:
                        fprintf(stderr, "The file '%s' is a socket and cannot be displayed, use '-h' for help.\n", argv[optind]);
                        break;
                    case S_IFBLK:
                        fprintf(stderr, "The file '%s' is a block device and cannot be displayed, use '-h' for help.\n", argv[optind]);
                        break;
                    case S_IFDIR:
                        fprintf(stderr, "The file '%s' is a directory and cannot be displayed, use '-h' for help.\n", argv[optind]);
                        break;
                    case S_IFCHR:
                        fprintf(stderr, "The file '%s' is a character device and cannot be displayed, use '-h' for help.\n", argv[optind]);
                        break;
                    case S_IFIFO:
                        fprintf(stderr, "The file '%s' is a named pipe and cannot be displayed, use '-h' for help.\n", argv[optind]);
                        break;
                }
    
                return 1;
            }

            //If the flag is set print filenames before each file.
            if (programFlags.showFilenameFlag == 1)
                fprintf(stdout, "-----%s-----\n", argv[optind]);

            displayFile(argv[optind], programFlags);
            optind++;
        }
        else {
            //We flush stdout to force anything left in the standard output to be printed before we print the error.
            fflush(stdout);

            fprintf(stderr, "\n\nThe file '%s' does not exist, use '-h' for help.\n", argv[optind]);
            return 1;
        }
    }

    return 0;
}


void displayFile(const char *filename, struct flagsStruct programFlags) {
    FILE *outputFile = fopen(filename, "r");
    char c;
    //Line display variables
    int lineCounter = 1;
    int printLineNumber = 0;
    int maxDigits;

    //If we are going to print the lines we have to pad them, for that we need the highest line number.
    if (programFlags.showLinesFlag == 1) {
        //Gets the number of characters needed to display the highest line. For this we count the lines in the file and use that as the max
        //characters.
        maxDigits = (int)floor(log10(countLines(filename)) + 1);
    }

    //Read the file on character at a time and 
    while ((c = fgetc(outputFile)) != EOF) {
        //Print line number
        if (printLineNumber == 0 && programFlags.showLinesFlag == 1) {
            //If we are marking line endings we add additional padding to accommodate them.
            if (programFlags.showEndsFlag == 1){
                fprintf(stdout, "  ");
            }

            fprintf(stdout, "%d", lineCounter);

            //We print enough spaces to reach "maxDigits".
            for (int i = 0; i < maxDigits - (int)floor(log10(lineCounter) + 1); i++) {
                fprintf(stdout, " ");
            }
            //Add padding after numbering
            fprintf(stdout, " ");

            lineCounter++;
            printLineNumber = 1;
        }

        //Detect newlines.
        if (c == '\n') {
            //Print line end indicator if the option is set.
            if (programFlags.showEndsFlag == 1) {
                fprintf(stdout, "$");
            }
            printLineNumber = 0;
        }

        //Print character.
        fprintf(stdout, "%c", c);
    }

    //If the flag is set add some bottom padding so filenames don't run into each other.
    if (programFlags.showFilenameFlag == 1)
        fprintf(stdout, "\n\n");
}


int countLines(const char *filename) {
    FILE *outputFile = fopen(filename, "r");
    char c;
    int lineCount = 0;

    while ((c = fgetc(outputFile)) != EOF) {
        //Print line number
        if (c == '\n')
            lineCount++;
    }

    return lineCount;
}