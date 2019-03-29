cd gyp
call ..\tools\gyp\src\gyp.bat -D target_arch=x64 --depth=. Air2.gyp --generator-output=../gyp_build -G msvs_version=2017
pause