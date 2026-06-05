#include "tesseract.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

// 문구 출력을 위한 가상 도구 (프로젝트 구조에 맞게 커스텀)
namespace PrettyCLI {
    void info(const std::string& msg) { std::cout << "[ℹ] INFO: " << msg << "\n"; }
    void error(const std::string& msg) { std::cout << "[❌] ERROR: " << msg << "\n"; }
}

namespace Tesseract {

    void run_simd_analyzer(const std::string& filename, const std::string& op) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            PrettyCLI::error("파일을 열 수 없습니다: " + filename);
            return;
        }

        std::string line;
        
        // ==========================================
        // 1. [새로운 도전] 행렬곱(matmul) 처리 로직
        // ==========================================
        if (op == "matmul") {
            std::vector<Matrix4x4> m1_list;
            std::vector<Matrix4x4> m2_list;

            // 행렬곱은 한 줄에 숫자 32개 (4x4 행렬 2개)를 읽어야 함!
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                std::stringstream ss(line);
                std::string val;
                
                Matrix4x4 m1, m2;
                float* m1_ptr = reinterpret_cast<float*>(&m1);
                float* m2_ptr = reinterpret_cast<float*>(&m2);

                // 앞의 행렬 m1 (숫자 16개) 파싱
                for (int i = 0; i < 16; ++i) {
                    if (std::getline(ss, val, ',')) *m1_ptr++ = std::stof(val);
                }
                // 뒤의 행렬 m2 (숫자 16개) 파싱
                for (int i = 0; i < 16; ++i) {
                    if (std::getline(ss, val, ',')) *m2_ptr++ = std::stof(val);
                }
                
                m1_list.push_back(m1);
                m2_list.push_back(m2);
            }

            size_t total_matrices = m1_list.size();
            PrettyCLI::info("총 " + std::to_string(total_matrices) + " 개의 4x4 행렬곱 연산 데이터 로드 완료.");

            std::vector<Matrix4x4> results(total_matrices);

            // ⏱️ 초정밀 하드웨어 타이머 가동!
            auto start = std::chrono::high_resolution_clock::now();

            for (size_t i = 0; i < total_matrices; ++i) {
                simd_matrix_mul(&results[i], &m1_list[i], &m2_list[i]);
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            // 보고서 출력
            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD MATRIX matmul             │\n";
            std::cout << "│ 처리 행렬 수 : " << total_matrices << " 개 (총 " << total_matrices * 32 << " 개 float)       │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 행렬당 속도: " << elapsed.count() / total_matrices << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 행렬곱 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        }

        // ==========================================
        // 2. 기존 벡터 사칙연산 로직 (add, sub, mul, div)
        // ==========================================
        std::vector<Vector4> v1_list;
        std::vector<Vector4> v2_list;

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string val;
            Vector4 v1, v2;

            if (std::getline(ss, val, ',')) v1.x = std::stof(val);
            if (std::getline(ss, val, ',')) v1.y = std::stof(val);
            if (std::getline(ss, val, ',')) v1.z = std::stof(val);
            if (std::getline(ss, val, ',')) v1.w = std::stof(val);
            if (std::getline(ss, val, ',')) v2.x = std::stof(val);
            if (std::getline(ss, val, ',')) v2.y = std::stof(val);
            if (std::getline(ss, val, ',')) v2.z = std::stof(val);
            if (std::getline(ss, val, ',')) v2.w = std::stof(val);

            v1_list.push_back(v1);
            v2_list.push_back(v2);
        }

        size_t total_vectors = v1_list.size();
        PrettyCLI::info("총 " + std::to_string(total_vectors) + " 개의 4D 벡터 연산 데이터 로드 완료.");
        std::vector<Vector4> results(total_vectors);

        auto start = std::chrono::high_resolution_clock::now();

        if (op == "add") {
            for (size_t i = 0; i < total_vectors; ++i) simd_vector_add(&results[i], &v1_list[i], &v2_list[i]);
        } else if (op == "sub") {
            for (size_t i = 0; i < total_vectors; ++i) simd_vector_sub(&results[i], &v1_list[i], &v2_list[i]);
        } else if (op == "mul") {
            for (size_t i = 0; i < total_vectors; ++i) simd_vector_mul(&results[i], &v1_list[i], &v2_list[i]);
        } else if (op == "div") {
            for (size_t i = 0; i < total_vectors; ++i) simd_vector_div(&results[i], &v1_list[i], &v2_list[i]);
        }else if (op == "dot") {
            // 내적 결과는 float(스칼라)이므로 float 배열을 준비!
            std::vector<float> dot_results(total_vectors);

            auto start = std::chrono::high_resolution_clock::now();

            for (size_t i = 0; i < total_vectors; ++i) {
                simd_vector_dot(&dot_results[i], &v1_list[i], &v2_list[i]);
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD VECTOR dot (내적)         │\n";
            std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 내적 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        }else if (op == "cross") {
            std::vector<Vector4> cross_results(total_vectors);

            auto start = std::chrono::high_resolution_clock::now();

            for (size_t i = 0; i < total_vectors; ++i) {
                simd_vector_cross(&cross_results[i], &v1_list[i], &v2_list[i]);
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD VECTOR cross (외적)       │\n";
            std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 외적 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> elapsed = end - start;

        std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
        std::cout << "│ 요청된 연산 : SIMD VECTOR " << op << "                         │\n";
        std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
        std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
        std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
        std::cout << "└────────────────────────────────────────────┘\n";
        std::cout << "[✔] SUCCESS: 분석 프로세스가 정상 종료되었습니다.\n";
    }
}