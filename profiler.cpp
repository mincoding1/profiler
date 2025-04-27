#include <iostream>
#include <string>
#include <windows.h>
using namespace std;

//============================================================================================================
//[경로 Check 필요!] 
//============================================================================================================
//빌드 프로그램 파일 경로 (MSBuild.exe)
const string msbuild_path = R"(C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe)";

//프로젝트 파일 경로 (target.vcxproj)
const string pjt_path = R"(C:\Users\user\source\repos\profiler\target\target.vcxproj)";

//빌드 후 실행해야 할 파일 경로 (target.exe)
const string exe_path = R"(C:\Users\user\source\repos\profiler\target\Release\target.exe)";
//============================================================================================================

void build() {
    //Release Mode로 Build
    const string option = R"(/p:Configuration=Release)";

    string cmd = string{ "\"" } + msbuild_path + string{ "\" " } + pjt_path + string{ " " } + option;
    system(cmd.c_str());
}

void run() {
    string cmd = string{ "\"" } + exe_path + string{ "\"" };
    system(cmd.c_str());
}

int main() {
    build();
    run();
}
