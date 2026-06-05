#ifndef PRETTYCLI_HPP
#define PRETTYCLI_HPP

#include <iostream>
#include <string>
#include <vector>

// 윈도우 환경일 때만 필요한 시스템 헤더 포함 (크로스 플랫폼 대응)
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace PrettyCLI {
    // ==========================================
    // 1. ANSI 이스케이프 시퀀스 (색상 및 스타일)
    // ==========================================
    const std::string RESET       = "\033[0m";
    const std::string BOLD        = "\033[1m";
    const std::string DIM         = "\033[2m";

    // 전경색 (글자색)
    const std::string RED         = "\033[31m";
    const std::string GREEN       = "\033[32m";
    const std::string YELLOW      = "\033[33m";
    const std::string BLUE        = "\033[34m";
    const std::string MAGENTA     = "\033[35m";
    const std::string CYAN        = "\033[36m";
    const std::string WHITE       = "\033[37m";

    // 커서 제어 시퀀스
    const std::string CLEAR_LINE  = "\033[2K\r"; // 현재 줄을 지우고 커서를 맨 앞으로

    // ==========================================
    // 2. OS 통합 터미널 초기화 함수 (UTF-8 & ANSI 활성화)
    // ==========================================
    inline void initTerminal() {
#if defined(_WIN32) || defined(_WIN64)
        // Windows 한글 깨짐 방지: 콘솔 출력/입력 인코딩을 UTF-8(65001)로 강제 설정
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        // Windows 10/11 가상 터미널(ANSI) 프로세싱 활성화
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
#endif
        // Linux(라즈베리파이) 및 macOS는 기본적으로 UTF-8과 ANSI를 지원하므로 별도 처리 없음
    }

    // ==========================================
    // 3. ASTRA 아스키 아트 로고 출력 함수
    // ==========================================
    inline void drawLogo() {
        std::cout << CYAN << BOLD << "    _       ____  _____  ____       _    " << "\n";
        std::cout << CYAN << BOLD << "   / \\     / ___||_   _||  _ \\     / \\   " << "\n";
        std::cout << BLUE << BOLD << "  / _ \\    \\___ \\  | |  | |_) |   / _ \\  " << "\n";
        std::cout << BLUE << BOLD << " / ___ \\    ___) | | |  |  _ <   / ___ \\ " << "\n";
        std::cout << BLUE << BOLD << "/_/   \\_\\  |____/  |_|  |_| \\_\\ /_/   \\_\\" << "\n";
        std::cout << RESET;
        std::cout << DIM << "=============================================" << RESET << "\n";
        std::cout << BOLD << " ASTRA " << RESET << ":: BLUSK Package Manager & Dev-Tools\n";
        std::cout << DIM << "=============================================" << RESET << "\n\n";
    }

    // ==========================================
    // 4. 기본 상태 로그 출력 함수
    // ==========================================
    inline void success(const std::string& msg) {
        std::cout << GREEN << BOLD << "[✔] SUCCESS: " << RESET << msg << "\n";
    }

    inline void error(const std::string& msg) {
        std::cerr << RED << BOLD << "[✘] ERROR: " << RESET << msg << "\n";
    }

    inline void info(const std::string& msg) {
        std::cout << BLUE << BOLD << "[ℹ] INFO: " << RESET << msg << "\n";
    }

    inline void warn(const std::string& msg) {
        std::cout << YELLOW << BOLD << "[⚠] WARN: " << RESET << msg << "\n";
    }

    // ==========================================
    // 5. 다이내믹 UI (진행 바 - 깜빡임 방지 최적화 버전)
    // ==========================================
    inline void drawProgressBar(const std::string& prefix, double progress, int barWidth = 30) {
        if (progress < 0.0) progress = 0.0;
        if (progress > 1.0) progress = 1.0;

        // 터미널에 바로 출력하지 않고, 메모리 상에서 문자열을 먼저 조립함
        std::string buffer = "";

        // \r : 커서를 맨 앞으로 이동
        // \033[2K : 현재 줄 깨끗이 청소
        buffer += "\r\033[2K";

        // 텍스트 및 조립 시작
        buffer += BOLD + prefix + RESET + " [";

        int pos = static_cast<int>(barWidth * progress);
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) {
                buffer += GREEN + "=" + RESET;
            } else if (i == pos && progress < 1.0) {
                buffer += GREEN + ">" + RESET;
            } else {
                buffer += " ";
            }
        }

        int percent = static_cast<int>(progress * 100.0);
        buffer += "] " + BOLD + std::to_string(percent) + "%" + RESET;

        // 완성된 버퍼를 단 한 번의 스트림 연산으로 터미널에 전송
        std::cout << buffer;
        std::cout.flush();
    }

    // ==========================================
    // 6. 구조화 UI (텍스트 박스 출력)
    // ==========================================
    inline void drawBox(const std::string& title, const std::vector<std::string>& lines) {
        size_t maxWidth = title.length();
        for (const auto& line : lines) {
            if (line.length() > maxWidth) maxWidth = line.length();
        }
        maxWidth += 4;

        std::cout << CYAN << "┌─ " << BOLD << WHITE << title << RESET << CYAN;
        for (size_t i = 0; i < maxWidth - title.length() - 2; ++i) std::cout << "─";
        std::cout << "┐\n" << RESET;

        for (const auto& line : lines) {
            std::cout << CYAN << "│ " << RESET << line;
            for (size_t i = 0; i < maxWidth - line.length(); ++i) std::cout << " ";
            std::cout << CYAN << " │\n" << RESET;
        }

        std::cout << CYAN << "└";
        for (size_t i = 0; i < maxWidth + 2; ++i) std::cout << "─";
        std::cout << "┘\n" << RESET;
    }
}

#endif // PRETTYCLI_HPP