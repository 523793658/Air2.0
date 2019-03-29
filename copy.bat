cd tools
junction -d D:/Air2.0Copy/assets
junction ../../Air2.0Copy/assets ../assets

junction -d D:/Air2.0Copy/gyp
junction ../../Air2.0Copy/gyp ../gyp

junction -d D:/Air2.0Copy/Shaders
junction ../../Air2.0Copy/Shaders ../Shaders

cd ../../Air2.0Copy
md source
cd ../Air2.0/tools

junction -d D:/Air2.0Copy/source/demo
junction ../../Air2.0Copy/source/demo ../source/demo

junction -d D:/Air2.0Copy/source/developer
junction ../../Air2.0Copy/source/developer ../source/developer

junction -d D:/Air2.0Copy/source/Editor
junction ../../Air2.0Copy/source/Editor ../source/Editor

junction -d D:/Air2.0Copy/source/runtime
junction ../../Air2.0Copy/source/runtime ../source/runtime

pause