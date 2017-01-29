# kppd
스냅800용 디스플레이 조절 바이너리

##빌드
Android NDK 받고
./build/tools/make_standalone_toolchain.py --arch arm --install-dir ..\arm

arm 폴더 생기면

../arm/bin/clang -pie repo/src/kppd.c repo/src/pp.c

## 왜 포크해서 만듬?
글쎄

##Installing on Android
adb push ./out/kppd /data/local/tmp/  
adb push ./template/postproc.conf /data/local/tmp/  
adb shell  
chmod 0755 /data/local/tmp/kppd  
./data/local/tmp/kppd /data/local/tmp/postproc.conf 

## 라이센스
GPL2