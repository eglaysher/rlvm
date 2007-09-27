@echo off
if "%RLDEV%" == "" goto no_rldev

:rldev
for %%d in (Module_Str_SEEN Module_Mem_SEEN Module_Jmp_SEEN Module_Sys_SEEN ExpressionTest_SEEN) do pushd "%%d" && (for %%f in (*.ke) do %RLDEV%\bin\rlc "%%f" -o SEEN0001.TXT && del %%~nf.TXT && %RLDEV%\bin\kprl -a %%~nf.TXT SEEN0001.TXT && del SEEN0001.TXT) & popd

setlocal enabledelayedexpansion 
SET RLFILES=
for %%d in (Module_Jmp_SEEN\farcall_withTest Module_Jmp_SEEN\farcallTest_0 Module_Jmp_SEEN\jumpTest Module_Sys_SEEN\SceneNum) do pushd "%%d" && (echo "" >SEEN.TXT && del SEEN.TXT && SET RLFILES= && (for %%f in (*.ke) do (SET RLFILES=!RLFILES! %%~nf.TXT && %RLDEV%\bin\rlc "%%f" -o %%~nf.TXT)) && %RLDEV%\bin\kprl -a SEEN.TXT !RLFILES!) & popd
endlocal

goto end

:no_rldev
echo "RLDEV environment variable is not set. Please configure your IDE to point this variable to the ad hoc value, or define it system-wide."

:end