all: codec_4061

codec_4061: codec_4061.c codec.c codec.h 
	gcc codec.c codec_4061.c -o codec_4061

clean:
	rm codec_4061  
