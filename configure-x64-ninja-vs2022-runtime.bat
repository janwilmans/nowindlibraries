set COM_BUILD="D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
set PRO_BUILD="D:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
set ENT_BUILD="D:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"

if exist %COM_BUILD% (
  call %COM_BUILD% x64
)

if exist %PRO_BUILD% (
  call %PRO_BUILD% x64
)

if exist %ENT_BUILD% (
  call %ENT_BUILD% x64
)
