#include "include/prettycli.hpp"
#include "include/tesseract.hpp"
#include <chrono>
#include <thread>
#include <iostream>
#include <string>

void show_help() {
    PrettyCLI::drawLogo();
    std::cout << "\n";
    std::cout << "┌─ Astra 하드웨어 가속 엔진 ─────────────────────────────────┐\n";
    std::cout << "│ 사용법:                                                  │\n";
    std::cout << "│                                                           │\n";
    std::cout << "│  ☆ SIMD 벡터/행렬 연산 (AoSoA + SIMD + Zero-Copy)        │\n";
    std::cout << "│     astra simd <파일.csv> <연산>                          │\n";
    std::cout << "│     연산: add, sub, mul, div, dot, cross, matmul         │\n";
    std::cout << "│                                                           │\n";
    std::cout << "│  ☆ Tesseract 엔진 정보                                   │\n";
    std::cout << "│     astra tessaract [옵션]                               │\n";
    std::cout << "│     옵션: info (기본정보), bench (벤치마크)               │\n";
    std::cout << "│                                                           │\n";
    std::cout << "│  ☆ 도움말                                                │\n";
    std::cout << "│     astra --help, astra -h                                │\n";
    std::cout << "└─────────────────────────────────────────────────────────┘\n";
}

int main(int argc, char* argv[]) {
    PrettyCLI::initTerminal();

    // 인자가 없거나 도움말 요청
    if (argc < 2) {
        show_help();
        return 0;
    }

    std::string cmd = argv[1];

    // ========================================
    // 1. 도움말 명령어
    // ========================================
    if (cmd == "--help" || cmd == "-h" || cmd == "help") {
        show_help();
        return 0;
    }

    // ========================================
    // 2. SIMD 벡터/행렬 연산 명령어
    // ========================================
    if (cmd == "simd") {
        if (argc < 4) {
            PrettyCLI::drawLogo();
            std::cout << "사용법: astra simd <파일경로.csv> <add|sub|mul|div|dot|cross|matmul>\n";
            std::cout << "예제:   astra simd data/vector.csv add\n";
            return 1;
        }

        std::string filename = argv[2];
        std::string operation = argv[3];

        Tesseract::run_simd_analyzer(filename, operation);
        return 0;
    }

    // ========================================
    // 3. Tesseract 엔진 정보 명령어
    // ========================================
    if (cmd == "tessaract") {
        std::string option = (argc >= 3) ? argv[2] : "info";

        PrettyCLI::drawLogo();
        std::cout << "\n";

        if (option == "info") {
            std::cout << "┌─ Tesseract 엔진 정보 ─────────────────────────────────────┐\n";
            std::cout << "│ Astra 하드웨어 가속 SIMD 연산 엔진                       │\n";
            std::cout << "│                                                           │\n";
            std::cout << "│ 기술 스택:                                              │\n";
            std::cout << "│  ├─ 구조: AoSoA (Array of Structures of Arrays)         │\n";
            std::cout << "│  ├─ 배치 크기: 8 (AVX2 = 256비트)                        │\n";
            std::cout << "│  ├─ 메모리: Zero-Copy 포인터 연산                        │\n";
            std::cout << "│  ├─ 정렬: 64바이트 캐시라인 정렬                         │\n";
            std::cout << "│  └─ 플랫폼: Windows(x86-64) + Linux(x86-64) + ARM64    │\n";
            std::cout << "│                                                           │\n";
            std::cout << "│ 지원 벡터 연산 (4D):                                    │\n";
            std::cout << "│  ├─ 기본: add, sub, mul, div                            │\n";
            std::cout << "│  └─ 수학: dot product, cross product                    │\n";
            std::cout << "│                                                           │\n";
            std::cout << "│ 지원 행렬 연산 (4x4):                                   │\n";
            std::cout << "│  └─ 행렬곱(matmul) - AoSoA 배치 처리                     │\n";
            std::cout << "│                                                           │\n";
            std::cout << "│ 성능 (9.5M 벡터 + 2.25M 행렬):                          │\n";
            std::cout << "│  ├─ 벡터: ~20 MB/s (0.44 M ops/s)                       │\n";
            std::cout << "│  └─ 행렬: ~22 MB/s (0.13 M ops/s)                       │\n";
            std::cout << "│                                                           │\n";
            std::cout << "│ 사용 예:                                                │\n";
            std::cout << "│  astra simd data/vector.csv add                         │\n";
            std::cout << "│  astra simd data/matrix.csv matmul                      │\n";
            std::cout << "└─────────────────────────────────────────────────────────┘\n";
        } else if (option == "bench") {
            std::cout << "⚠️  벤치마크 모드 - 준비 중\n";
        } else {
            std::cout << "알 수 없는 옵션: " << option << "\n";
            std::cout << "사용 가능한 옵션: info, bench\n";
            return 1;
        }

        return 0;
    }

    // ========================================
    // 4. 알 수 없는 명령어
    // ========================================
    std::cout << "❌ 알 수 없는 명령어: " << cmd << "\n\n";
    show_help();
    return 1;
}