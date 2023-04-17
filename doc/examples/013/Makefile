INCLUDE_PATH = $(FF_INC_PATH)
LIB_PATH = $(FF_LIB_PATH)

LIBS = -lavcodec -lavformat -lswscale -lpostproc -lavutil -lm

all: demo_scale
scale_image.o: scale_image.c
	gcc -c scale_image.c -o scale_image.o -I$(INCLUDE_PATH)
demo_scale: scale_image.o
	gcc -g -o demo_scale scale_image.o -L$(LIB_PATH) $(LIBS)
.PHONY: clean all
clean:
	rm -f demo_scale *.o
