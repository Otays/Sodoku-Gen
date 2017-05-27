##########################################################
#	Sodoku Makefile
#		Use "CC = g++" for serial code
#		Use "CC = nvcc" for CUDA code
#
#	nvcc -c fractal_hyb.cu -o CUfractal.o
##########################################################

CC = nvcc.exe
CFLAGS =-w -I. -O3 -arch=sm_35

sodoku: sodoku.o colorlogs.o
	$(CC) $(CFLAGS) -o sodoku_gen.exe sodoku.o colorlogs.o

colorlogs.o: colorlogs.h colorlogs.c 
	$(CC)  $(CFLAGS) -c colorlogs.c
	
sodoku.o: sodoku.cu
	$(CC)  $(CFLAGS) -c sodoku.cu

run:
	./mcc < test.mc
