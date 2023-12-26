#!/bin/bash

#--------------------------------------------
# author: Tony Song <songshaoying@kuaishou.com>

# 测试环境相关安装包版本如下：
# os centos/ubuntu
# TensorFlow 2.6.0
# CUDA 11.2.2
# CUDNN 8.1.1.33
# NVIDIA Driver GeForce RTX 2060 525.85.12

# 安装包下载和相关对照表网站：
# TensorFlow、CUDA和CUDNN版本对照表 https://tensorflow.google.cn/install/source?hl=zh-cn#gpu
# TensorFlow C语言库 https://tensorflow.google.cn/install/lang_c
# CUDNN 安装包 https://developer.nvidia.cn/rdp/cudnn-download
# CUDA 安装包 https://developer.nvidia.cn/cuda-downloads
# NVIDIA 驱动和CUDA版本对照表 https://docs.nvidia.com/cuda/cuda-toolkit-release-notes/index.html
# NVIDIA 驱动安装包 https://www.nvidia.com/Download/index.aspx
# DNN模型和测试文件 https://github.com/guoyejun/dnn_processing

#--------------------------------------------
install_check(){
    if [ $1 -ne 0 ];then
        echo_log 0 "$2 失败"
        exit
    fi

    echo
}

echo_log(){
    case $1 in
    0) echo -e "\033[31m$2\033[0m";;
    1) echo -e "\033[32m$2\033[0m";;
    2) echo -e "\033[33m$2\033[0m";;
    esac
}

echo_log 1 "#--------------------------------------------
重要：安装NVIDIA DNN环境，需要安装TensorFlow、CUDA、CUDNN、NVIDIA Driver，并且要求相互之间版本匹配
版本依赖关系如下：TensorFlow -> CUDA|CUDNN -> NVIDIA Driver
先确定TensorFlow C库版本，再确定其他安装包版本，安装顺序则相反
#--------------------------------------------
"

echo_log 2 "###### 1、执行脚本前，需要从NVIDIA官网下载安装NVIDIA显卡型号对应版本驱动  ######"
nvidia-smi

install_check $? '验证 NVIDIA显卡驱动'

echo_log 2 "###### 2、下载安装 CUDA ######"
wget https://developer.download.nvidia.com/compute/cuda/11.2.2/local_installers/cuda_11.2.2_460.32.03_linux.run
bash cuda_11.2.2_460.32.03_linux.run --no-man-page --override --silent \
  --toolkit --toolkitpath=/usr/local/cuda-11.2 --librarypath=/usr/local/cuda-11.2
CUDA_VERSION=`nvcc --version|grep tools |awk -F ' |,' '{print $6}'`

install_check $? '安装 CUDA'

echo_log 2 "CUDA version $CUDA_VERSION"
echo "/usr/local/cuda-11.2/targets/x86_64-linux/lib" >> /etc/ld.so.conf
echo

echo_log 2 "###### 3、下载安装 CUDNN ######"
wget https://developer.download.nvidia.com/compute/redist/cudnn/v8.1.1/cudnn-11.2-linux-x64-v8.1.1.33.tgz
mkdir /usr/local/cudnn-11.2-8.1.1.33
tar -xzf cudnn-11.2-linux-x64-v8.1.1.33.tgz -C /usr/local/cudnn-11.2-8.1.1.33

install_check $? '安装 CUDNN'

CUDNN_MAJOR=`cat /usr/local/cudnn-11.2-8.1.1.33/cuda/include/cudnn_version.h | grep CUDNN_MAJOR | grep -v CUDNN_VERSION | awk '{print $3}'`
CUDNN_MINOR=`cat /usr/local/cudnn-11.2-8.1.1.33/cuda/include/cudnn_version.h | grep CUDNN_MINOR | grep -v CUDNN_VERSION | awk '{print $3}'`
echo_log 2 "CUDNN  version $CUDNN_MAJOR.$CUDNN_MINOR"

echo "/usr/local/cudnn-11.2-8.1.1.33/cuda/lib64" >> /etc/ld.so.conf
echo

echo_log 2 "###### 4、下载安装 TensorFlow ######"
#必须GPU版本C语言库
wget https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-gpu-linux-x86_64-2.6.0.tar.gz
tar -xzf libtensorflow-gpu-linux-x86_64-2.6.0.tar.gz -C /usr/local
echo "/usr/local/lib" >> /etc/ld.so.conf

install_check $? '安装 TensorFlow'

ldconfig

# 验证 TensorFlow 环境，非必须步骤
cat >tf.c<<EOF
#include <stdio.h>
#include <tensorflow/c/c_api.h>
int main() {
    printf("\033[33mTensorFlow C library version %s\033[0m\n", TF_Version());
    return 0;
}
EOF

gcc tf.c -ltensorflow -o hello_tf
./hello_tf
install_check $? '验证 TensorFlow'

echo_log 2 "###### 5、重新编译 FFmpeg ######"
##指定--enable-libtensorflow 参数
git clone https://git.ffmpeg.org/ffmpeg.git -b release/6.0
cd ffmpeg
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
./configure --enable-libx264 --enable-gpl --enable-nonfree --enable-libtensorflow --extra-libs='-lc -lm -lbz2 -ldl -lpthread -lstdc++' && make -j8 && make install

install_check $? '编译 FFmpeg'

echo_log 2 "###### 6、下载 DNN 模型及测试文件，测试FFmpeg DNN 滤镜 ######"
# 也可以使用 https://github.com/HighVoltageRocknRoll/sr，在TensorFlow环境里生成模型
# git clone https://github.com/HighVoltageRocknRoll/sr
# docker pull tensorflow/tensorflow:devel
# docker run -it -w /tensorflow_src -v $PWD:/sr -e HOST_PERMS="$(id -u):$(id -g)" tensorflow/tensorflow:1.15.5
# cd /sr
# python generate_header_and_model.py --model=srcnn --ckpt_path=checkpoints/srcnn/
# python generate_header_and_model.py --model=espcn --ckpt_path=checkpoints/espcn/

git clone https://github.com/guoyejun/dnn_processing
ffmpeg -i dnn_processing/models/480p.jpg -vf format=yuv420p,scale=w=iw*2:h=ih*2,dnn_processing=dnn_backend=tensorflow:model=dnn_processing/models/srcnn.pb:input=x:output=y srcnn.jpg
ffmpeg -i dnn_processing/models/480p.jpg -vf format=yuv420p,dnn_processing=dnn_backend=tensorflow:model=dnn_processing/models/espcn.pb:input=x:output=y espcn.jpg