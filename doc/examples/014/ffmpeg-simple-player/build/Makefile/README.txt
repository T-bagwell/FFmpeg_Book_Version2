在Linux或者mac系统上，可以使用make工具进行直接编译。

使用方式如下：
make FF_INC=-I/your_ffmpeg_dev_path/include/      FF_LIB=-L/your_ffmpeg_dev_path/lib/      SDL_INC=-I/your_SDL_dev_path/SDL2/      SDL_LIB=-L/your_SDL_dev_path

说明：
FF_INC 指定了预编译好的ffmpeg头文件路径
FF_LIB  指定了预编译好的ffmpeg库路径

SDL_INC 指定了预编译好的SDL头文件路径
SDL_LIB  指定了预编译好的SDL库路径


