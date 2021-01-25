#!/bin/sh

if [ ! -d "./build/" ];then
  echo "[INFO]>>> 创建build文件夹"
  mkdir ./build
else
  echo "[INFO]>>> 清空build下内容"
  rm -rf build/*
fi

cd build
cmake ..
make -j8
mv idcard ..
cd ..
./idcard
