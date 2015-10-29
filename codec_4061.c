#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "codec.h"

#define MAX_LINES 100
#define MAX_PATH_SIZE 100
#define ERROR_CODE 1


void encode_dir(char *dir, char inpath[], char outpath[], FILE *report);

void decode_dir(char *dir, char inpath[], char outpath[], FILE *report);

void sortReport(char *reportFile);

int compare(const void *ap, const void *bp);


int main(int argc, char* argv[]) {

	if(argc != 4) {
		printf ("Usage: -[ed] <input_directory> <output_directory>\n") ;
		return 1 ;
	}
    //test print dir

    char *cwd;
    cwd = getcwd(0, 0);
    if(!cwd) {
        fprintf(stderr, "getcwd failed: %s\n", strerror (errno));
    }
    
    char inpath[MAX_PATH_SIZE] ;
    char outpath[MAX_PATH_SIZE];
    strcpy(inpath, cwd);
    strcpy(outpath, cwd);
    strcat(inpath, "/");
    strcat(inpath, argv[2]);
    strcat(outpath, "/");
    strcat(outpath, argv[3]);
    
	//create the 'output'directory
    int retval = mkdir(outpath, 0755);
    if(retval == -1) {
		fprintf(stderr, "mkdir() failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}



    //create the report file directory
	
	FILE *report;
    char reportPath[MAX_PATH_SIZE];
    strcpy(reportPath, outpath);
    strcat(reportPath, "/");
    strcat(reportPath, argv[2]);
    strcat(reportPath, "_report.txt");
	if((report = fopen(reportPath, "w")) == NULL) {
		perror("failed to open the report file\n");
	}
	
	//decide whether to encode or decode
	if(strcmp(argv[1], "-e") == 0) {
        encode_dir(argv[2],inpath,outpath,report);
	}
	else if(strcmp(argv[1], "-d") == 0) {
        decode_dir(argv[2],inpath,outpath,report);
	}
	else {
		perror("error command for encoding or decoding");
	} 
	
	if( fclose(report) == -1) {
		perror("report file close error.\n");
		exit(ERROR_CODE);
	}

	//sort the report file
	sortReport(reportPath);
	return (0);

}

void encode_dir(char *dir, char inpath[], char outpath[], FILE *report)
{
    DIR *dp;
	FILE *in_fp, *out_fp;
    struct dirent *entry;
    struct stat statbuf;
    int before_encode, after_encode, quotient, reminder;
	uint8_t output[4];
    uint8_t input[3];
    int flag = 0;

        
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }

    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
                continue;
            /* Recurse at a new indent level */
            char new_inpath[MAX_PATH_SIZE];
            char new_outpath[MAX_PATH_SIZE];
            strcpy(new_inpath, inpath);
            strcpy(new_outpath, outpath);
            strcat(new_inpath, "/");
            strcat(new_inpath, entry->d_name);
            strcat(new_outpath, "/");
            strcat(new_outpath, entry->d_name);
            mkdir(new_outpath, 0755);
	        fprintf(report, "%s%s%d%s%d\n", entry->d_name, ", directory, ", 0,", ",  0);	
            encode_dir(entry->d_name,new_inpath,new_outpath, report);
        }
        else {
            //printf("%*s%s\n",spaces,"",entry->d_name);
            
            char temp[MAX_PATH_SIZE];
            char in_temp[MAX_PATH_SIZE];
            strcpy(temp, outpath);
            strcpy(in_temp, inpath);
            strcat(temp, "/");
            strcat(in_temp, "/");
            strcat(temp, entry->d_name);
            strcat(in_temp, entry->d_name);
            if((in_fp = fopen(in_temp, "r")) == NULL) {
            	perror("failed to copy the file\n");
            }
            if((out_fp = fopen(temp, "w")) == NULL) {
            	perror("failed to copy the file\n");
            }
            before_encode = statbuf.st_size;
            if(before_encode == 1) {
                after_encode = 1;
            } else {
                quotient = before_encode / 3;
                reminder = before_encode % 3;
                
                after_encode = 4 * quotient;
                
                int i;
                for(i=0; i<quotient; i++) {
		    		flag = 1;
                    fread(input, sizeof(uint8_t), 3, in_fp);
                    size_t length = encode_block(input, output, 3);
		    	    fwrite(output, sizeof(uint8_t), length, out_fp);
		    	    

                }
                if(reminder > 0) {
                    flag = 1;
		    	    fread(input, sizeof(uint8_t), reminder, in_fp);
		    	    //fill the rest of the elements of the array with 0
		    	    uint8_t ch = 0;
		    	    uint8_t reminder_str[3];
                    int j;
                    for(j=0; j<reminder; j++) {
                        reminder_str[j] = input[j];
                    }
		    	    reminder_str[j] = ch;
                    
		    	    encode_block(reminder_str, output, reminder);
		    	    fwrite(output, sizeof(uint8_t), 4, out_fp);
		    	    
		    	    after_encode = after_encode + 4;
		    	}
                //add the newline to the output file
                if(flag == 1) {
                    unsigned char new_line = 0x0a;
		    	    fputc(new_line, out_fp); 
		    	    after_encode++;
                    flag = 0;
                }
		    	
		    						
	            fprintf(report, "%s%s%d%s%d\n", entry->d_name, ", regular file, ", before_encode,", ",  after_encode);	
		    	
                fclose(in_fp);
		    	fclose(out_fp);	

            }
        } 
    }
    chdir("..");

    closedir(dp);
}

void decode_dir(char *dir, char inpath[], char outpath[], FILE *report)
{
    DIR *dp;
	FILE *in_fp, *out_fp;
    struct dirent *entry;
    struct stat statbuf;
    int before_decode, after_decode, quotient, reminder;
	uint8_t output[3];
    uint8_t input[4];
    int flag = 0;

        
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }

    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0)
                continue;
            /* Recurse at a new indent level */
            char new_inpath[MAX_PATH_SIZE];
            char new_outpath[MAX_PATH_SIZE];
            strcpy(new_inpath, inpath);
            strcpy(new_outpath, outpath);
            strcat(new_inpath, "/");
            strcat(new_inpath, entry->d_name);
            strcat(new_outpath, "/");
            strcat(new_outpath, entry->d_name);
            mkdir(new_outpath, 0755);
	        fprintf(report, "%s%s%d%s%d\n", entry->d_name, ", directory, ", 0,", ",  0);	
            decode_dir(entry->d_name,new_inpath,new_outpath,report);
        }
        else {
            
            char temp[MAX_PATH_SIZE];
            char in_temp[MAX_PATH_SIZE];
            strcpy(temp, outpath);
            strcpy(in_temp, inpath);
            strcat(temp, "/");
            strcat(in_temp, "/");
            strcat(temp, entry->d_name);
            strcat(in_temp, entry->d_name);
            if((in_fp = fopen(in_temp, "r")) == NULL) {
            	perror("failed to copy the file\n");
            }
            if((out_fp = fopen(temp, "w")) == NULL) {
            	perror("failed to copy the file\n");
            }
            before_decode = statbuf.st_size;
            if(before_decode == 1) {
                after_decode = 1;
            } else {
                quotient = before_decode / 4;
                int i;
                after_decode = 3 * quotient;
                for(i=0; i<quotient; i++) {
                    fread(input, sizeof(uint8_t), 4, in_fp);
                    size_t length = decode_block(input, output);
		    	    fwrite(output, sizeof(uint8_t), length, out_fp);
		    	    
                }
                
	            fprintf(report, "%s%s%d%s%d\n", entry->d_name, ", regular file, ", before_decode,", ",  after_decode);	
		        fclose(in_fp);
		        fclose(out_fp);	

            }
        } 
    }
    chdir("..");

    closedir(dp);
}

void sortReport(char *reportFile) {
	FILE *in = fopen(reportFile, "r");
    if (!in)
        exit(ERROR_CODE);

    char **pbuf, **buf;
    pbuf = buf = malloc(sizeof (char *) * MAX_LINES);

    size_t count = 0, len = 0;
    while (getline(pbuf, &len, in) != -1) {
        pbuf++;
        count++;
        len = 0;
    }

    qsort(buf, count, sizeof(char *), compare);
    /* cleanup and print or whatever */
	//write the sorted contents to report file from the "contents array"
	if( fclose(in) == -1) {
		perror("report file close error.\n");
		exit(ERROR_CODE);
	}
	
    FILE *out = fopen(reportFile, "w");
    if (!out)
        exit(ERROR_CODE);
    
    int i;
    for(i=0; i<count; i++) {
		fputs(buf[i], out);
	}
	if( fclose(out) == -1) {
		perror("report file close error.\n");
		exit(ERROR_CODE);
	}
}
	
int compare(const void *ap, const void *bp)
{
    char **a = (char **)ap;
    char **b = (char **)bp;

    return strcmp(*a, *b);
}
