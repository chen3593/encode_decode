name: Raoyin Chen, Hezhi Wang
x500: chen3593, wang5575
CSELabs machine: lind40-27

The purpose of the program:

	Our main program c​odec_4061 w​ill be responsible for encoding/decoding all files present in the given input directory using the given scheme.

How to compile the program:

	Simply use the makefile or run gcc codec.c codec_4061.c -o codec_4061

How to use the program from the shell (syntax):

	Usage: -[ed] <input_directory> <output_directory>

­What exactly your program does:

	Our main program first parse the command line and decide whether to encode or decode. Then recursively traverse the input directory and encode/decode each file it encounters. The encoded/decoded files will be placed in the output directory, and will have the same name as the original file. Additionally, a report file will be generated for each run of our program.
