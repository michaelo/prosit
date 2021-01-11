call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\vsdevcmd\core\vsdevcmd_start.bat"

meson build --backend=vs
cd build
msbuild prosit.sln
