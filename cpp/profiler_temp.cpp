//=============================================================================================================================
#include <string>

//빌드 프로그램 파일 경로 (MSBuild.exe)
const std::string msbuild_path = R"(C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe)";

//솔루션 경로 (필요시 본인에게 맞게 경로 수정 필요)
const std::string solution_path = R"(C:\Users\minco\source\repos\Solution98\)";
//=============================================================================================================================


#include <iostream>
#include <windows.h>

const std::string pjt_path = solution_path + R"(Target\Target.vcxproj)"; // 빌드 할 프로젝트 파일 경로 (target.vcxproj)
const std::string exe_path = solution_path + R"(Target\Release\Target.exe)"; // 빌드 후 실행해야 할 파일 경로 (target.exe)

void build() {
    //Release Mode로 Build
    const std::string option = R"(/p:Configuration=Release)";

    std::string cmd = std::string{ "\"" } + msbuild_path + std::string{ "\" " } + pjt_path + std::string{ " " } + option;
    system(cmd.c_str());
}

void run() {
    std::string cmd = std::string{ "\"" } + exe_path + std::string{ "\"" };
    system(cmd.c_str());
}

int main() {
    build();
    run();
}
