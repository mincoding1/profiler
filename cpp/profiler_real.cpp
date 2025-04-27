#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <regex>
#include <map>
#include <iomanip>
#include <cstdio>


using namespace std;

//============================================================================================================
//[경로 Check 필요!] 
//============================================================================================================
//빌드 프로그램 파일 경로 (MSBuild.exe)
const string msbuild_path = R"(C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe)";

//프로젝트 파일 경로 (target.vcxproj)
const string pjt_path = R"(C:\Users\user\source\repos\profiler\latency\latency.vcxproj)";

//빌드 후 실행해야 할 파일 경로 (target.exe)
const string exe_path = R"(C:\Users\user\source\repos\profiler\latency\Release\latency.exe)";

//JSON LINT CPP 파일 경로
const string cpp_path = R"(C:\Users\user\source\repos\profiler\target\json_lint_preprocessing.cpp)";

//Output cpp 파일 경로
const string out_path = R"(C:\Users\user\source\repos\profiler\latency\generated.cpp)";

//Output log 파일 경로
const string log_path = R"(C:\Users\user\source\repos\profiler\generator\latency_log.txt)";

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

//=============================================================================================================

string fileStr;

std::vector<std::string> insertLatencyCode() {

    ifstream in{ cpp_path };

    std::ostringstream ss;
    ss << in.rdbuf();
    std::string fileContent = ss.str();

    // 테스트
    //std::cout << fileContent << std::endl;

    // 한 줄씩 분리
    std::vector<std::string> lines;
    std::string line;
    std::istringstream iss(fileContent);
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    //준비
    std::vector<std::string> output;
    output.push_back("#define LATENCY_CHECK_START(funcName) latencyCheckStart(funcName)");
    output.push_back("#define LATENCY_CHECK_END(funcName)   latencyCheckEnd(funcName)");
    output.push_back(""); // 빈 줄 구분

    output.push_back("#include <chrono>");
    output.push_back("#include <map>");
    output.push_back("#include <string>");
    output.push_back("#include <mutex>");
    output.push_back("#include <fstream>");
    output.push_back("#include <iostream>");
    output.push_back("");
    output.push_back("using Clock = std::chrono::steady_clock;");
    output.push_back("using TimePoint = std::chrono::time_point<Clock>;");
    output.push_back("");
    output.push_back("static std::map<std::string, TimePoint> g_latencyStartTime;");
    output.push_back("static std::mutex g_latencyMutex;");
    output.push_back("");
    output.push_back("void latencyCheckStart(const std::string& funcName) {");
    output.push_back("    std::lock_guard<std::mutex> lock(g_latencyMutex);");
    output.push_back("    g_latencyStartTime[funcName] = Clock::now();");
    output.push_back("}");
    output.push_back("");
    output.push_back("void latencyCheckEnd(const std::string& funcName) {");
    output.push_back("    std::lock_guard<std::mutex> lock(g_latencyMutex);");
    output.push_back("    auto it = g_latencyStartTime.find(funcName);");
    output.push_back("    if (it == g_latencyStartTime.end()) {");
    output.push_back("        std::cerr << \"[latencyCheckEnd] No start record for \" << funcName << std::endl;");
    output.push_back("        return;");
    output.push_back("    }");
    output.push_back("    auto start = it->second;");
    output.push_back("    auto end = Clock::now();");
    output.push_back("    auto latencyUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();");
    output.push_back("    std::ofstream logfile(\"latency_log.txt\", std::ios::app);");
    output.push_back("    if (logfile.is_open()) {");
    output.push_back("        logfile << \"[LATENCY] \" << funcName << \": \" << latencyUs << \" us\" << std::endl;");
    output.push_back("        logfile.close();");
    output.push_back("    } else {");
    output.push_back("        std::cerr << \"로그파일 열기 실패\\n\";");
    output.push_back("    }");
    output.push_back("    g_latencyStartTime.erase(it);");
    output.push_back("}");


    std::regex func_call_regex(R"(\b(\w+)\s*\(.*\)\s*;)");
    std::regex func_decl_regex(R"(^\s*\w[\w\s\*]*\s+\w+\s*\([^;{]*\)\s*[;{]\s*$)");
    std::regex for_while_regex(R"(^\s*(for|while)\b)");

    // 결과 저장
    for (const auto& line : lines) {
        std::smatch match;

        // for/while 문이면 그냥 통과
        if (std::regex_search(line, for_while_regex)) {
            output.push_back(line);
            continue;
        }

        // 함수 호출이고, 선언/정의가 아닌 경우만 체크
        if (std::regex_search(line, match, func_call_regex) &&
            !std::regex_search(line, func_decl_regex)) {
            std::string funcName = match[1].str();
            // memset, printf는 제외
            if (funcName == "memset" || funcName == "printf" || funcName == "strlen")
            {
                output.push_back(line);
                continue;
            }
            output.push_back("LATENCY_CHECK_START(\"" + funcName + "\");");
            output.push_back(line);
            output.push_back("LATENCY_CHECK_END(\"" + funcName + "\");");
        }
        else {
            output.push_back(line);
        }
    }

    // 출력
    for (const auto& outLine : output) {
        std::cout << outLine << "\n";
    }

    return output;

}

void saveOutputToFile(const std::vector<std::string>& output) {
    std::ofstream ofs(out_path);

    for (const auto& line : output) {
        ofs << line << "\n";
    }
}

void chart() {
    std::ifstream in{ log_path };

    std::ostringstream ss;
    ss << in.rdbuf();
    std::string fileContent = ss.str();

    std::istringstream iss(fileContent);
    std::string line;

    std::map<std::string, int> latencySum;

    while (std::getline(iss, line)) {
        auto p1 = line.find("] ");
        auto p2 = line.find(": ");
        auto p3 = line.find(" us");
        if (p1 == std::string::npos || p2 == std::string::npos || p3 == std::string::npos)
            continue;

        std::string func = line.substr(p1 + 2, p2 - (p1 + 2));
        int us = std::stoi(line.substr(p2 + 2, p3 - (p2 + 2)));
        latencySum[func] += us;
    }

    // log 스케일에 사용할 최대 별 개수
    const int MAX_STAR = 40;

    // 가장 큰 누적 us 값 찾기
    int max_us = 0;
    for (const auto& elem : latencySum) {
        if (elem.second > max_us)
            max_us = elem.second;
    }

    // log10(1) = 0 방지. 최소값 1로 설정
    double max_log = max_us > 0 ? std::log10(static_cast<double>(max_us)) : 1.0;

    std::cout << "\n== LATENCY CHART (log scale, us 단위) ==\n";
    for (const auto& elem : latencySum) {
        const std::string& func = elem.first;
        int us = elem.second;

        // log10(us) / log10(max_us) * MAX_STAR
        int starCnt = 0;
        if (us > 0 && max_log > 0)
            starCnt = static_cast<int>(std::round((std::log10(static_cast<double>(us)) / max_log) * MAX_STAR));

        std::cout << std::setw(16) << std::left << func << ": ";
        for (int i = 0; i < starCnt; ++i)
            std::cout << "*";
        std::cout << " (" << us << " us)\n";
    }
}

int main() {
    std::vector<std::string> output = insertLatencyCode();
    
    saveOutputToFile(output);

    build();

    // 기존 latency log파일 삭제
    std::remove(log_path.c_str());

    run();

    chart();

}
