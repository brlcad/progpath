@echo off
setlocal EnableExtensions DisableDelayedExpansion

if "%~1"=="" exit /b 1
pushd "%~1" || exit /b 1
set "expected_tests="
if exist expected-test-count.txt set /p expected_tests=<expected-test-count.txt
if not defined expected_tests (
  echo ERROR: missing expected test count
  exit /b 1
)
if exist tests\utf8-staging (
  restore-reactos-utf8.exe || exit /b 1
)
set "tests_run=0"

rem Discover binaries so newly added executable tests are included automatically.
for /r tests %%t in (test_*.exe) do (
  call :run_test "%%t" || exit /b 1
  set /a tests_run+=1 >nul
)
if "%tests_run%"=="0" (
  echo ERROR: no test executables found
  exit /b 1
)
if not "%tests_run%"=="%expected_tests%" (
  echo ERROR: expected %expected_tests% test executables, ran %tests_run%
  exit /b 1
)

set "PWD=%CD%"
progpath.exe >demo-output.txt 2>&1
set "demo_result=%errorlevel%"
type demo-output.txt
if not "%demo_result%"=="0" exit /b 1
find "ERROR:" demo-output.txt >nul
if errorlevel 2 exit /b 1
if not errorlevel 1 exit /b 1
rem ReactOS findstr lacks regex support; check the exact absolute path instead.
find /i "Program executable is [ %CD%\progpath.exe ]" demo-output.txt >nul || exit /b 1
echo PASS: %tests_run% test executables and demo output
popd
exit /b 0

:run_test
setlocal
pushd "%~dp1" || exit /b 1
rem Match CTest's startup directory, including the Unicode test directory.
set "PWD=%CD%"
if /i "%~nx1"=="test_method_priority.exe" set "PROGPATH_DEBUG=0"
echo Running %~f1
"%~f1"
set "test_result=%errorlevel%"
popd
endlocal & exit /b %test_result%
