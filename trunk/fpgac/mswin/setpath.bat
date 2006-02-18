set MY_STUFF=d:\
set prog_stuff=d:
set UNIX_STUFF=d:\usr\local\wbin;d:\usr\local\lib;d:\usr\lib
set PATH_WBEM=%WINDIR%\system32;%WINDIR%;%WINDIR%\System32\Wbem

rem DEVCPP
rem set CC_HOME=%PROG_STUFF%\devcpp

rem MINGW
set CC_HOME=%PROG_STUFF%\mingw

rem BCC
rem set CC_HOME=%PROG_STUFF%\Program Files\bcc5
rem set INCLUDE=%CC_HOME%\include

rem MSVC
rem set CC_HOME=%PROG_STUFF%\progra~1\micros~1
rem set INCLUDE=%PROG_STUFF%\progra~1\micros~1\include

rem MINGW, DEVCPP
set PATH_CCBIN=%CC_HOME%\bin;%CC_HOME%\libexec\gcc\mingw32\3.4.2

rem BCC, MSVC
rem set PATH_CCBIN=%CC_HOME%\bin;

SET PATH_ME=%PATH_CCBIN%;%UNIX_STUFF%
set PATH=%PATH_ME%;%PATH_WBEM%;

set BISON_SIMPLE=d:\usr\local\share\bison.simple
set BISON_HAIRY=d:\usr\local\share\bison.hairy
