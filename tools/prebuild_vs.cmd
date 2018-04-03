echo | set /p dummyName="#define GIT_TAG " > ..\src\git_tag.h
git -C ..\src describe --tags --long >> ..\src\git_tag.h
echo #define PRJ_TAG %1 >> ..\src\git_tag.h