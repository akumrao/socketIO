# socketIO
C++ socketIO Signaller for all the plateform

It works with Android, IOS and windows and linux

I am using it at media streaming, please check mediaserver respository.

# socketIO
C++ socketIO Signaller for all the plateform

It works with Android, IOS and windows and linux

I am using it at media streaming, please check mediaserver respository.


If compiling as a part of webrtc then do


gn gen out/x64/Debug --args="is_debug=true use_rtti=true target_cpu=\"x64\""
ninja -C out/x64/Debug signaler

