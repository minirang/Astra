#include "tesseract.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <cstring>

namespace PrettyCLI {
    void info(const std::string& msg) { std::cout << "[ℹ] INFO: " << msg << "\n"; }
    void error(const std::string& msg) { std::cout << "[❌] ERROR: " << msg << "\n"; }
}

namespace Tesseract {

    // ========== AoSoA 변환 헬퍼 함수 ==========
    void convert_aos_to_aosoa(const std::vector<Vector4>& aos_data, AoSoAVector4& aosoa) {
        const size_t BATCH = AoSoAVector4::BATCH_SIZE;
        float* xs = aosoa.get_xs();
        float* ys = aosoa.get_ys();
        float* zs = aosoa.get_zs();
        float* ws = aosoa.get_ws();
        
        for (size_t i = 0; i < aos_data.size(); ++i) {
            size_t local_idx = i % BATCH;
            size_t batch_offset = (i / BATCH) * BATCH;
            
            xs[batch_offset + local_idx] = aos_data[i].x;
            ys[batch_offset + local_idx] = aos_data[i].y;
            zs[batch_offset + local_idx] = aos_data[i].z;
            ws[batch_offset + local_idx] = aos_data[i].w;
        }
    }

    void convert_aosoa_to_aos(const AoSoAVector4& aosoa, std::vector<Vector4>& aos_data) {
        const size_t BATCH = AoSoAVector4::BATCH_SIZE;
        aos_data.resize(aosoa.total_count);
        
        const float* xs = aosoa.get_xs();
        const float* ys = aosoa.get_ys();
        const float* zs = aosoa.get_zs();
        const float* ws = aosoa.get_ws();
        
        for (size_t i = 0; i < aosoa.total_count; ++i) {
            size_t batch_offset = (i / BATCH) * BATCH;
            size_t local_idx = i % BATCH;
            aos_data[i] = {xs[batch_offset + local_idx], 
                          ys[batch_offset + local_idx],
                          zs[batch_offset + local_idx],
                          ws[batch_offset + local_idx]};
        }
    }

    // 행렬 AoSoA 변환
    void convert_aos_matrix_to_aosoa(const std::vector<Matrix4x4>& aos_matrices, AoSoAMatrix4x4& aosoa) {
        const size_t BATCH = AoSoAMatrix4x4::BATCH_SIZE;
        
        for (int row = 0; row < 4; ++row) {
            float* row_data = aosoa.get_row(row);
            
            for (size_t i = 0; i < aos_matrices.size(); ++i) {
                size_t local_idx = i % BATCH;
                size_t batch_idx = i / BATCH;
                
                float* elem = &row_data[batch_idx * BATCH * 4 + local_idx * 4];
                std::memcpy(elem, &aos_matrices[i].rows[row], sizeof(Vector4));
            }
        }
    }

    void run_simd_analyzer(const std::string& filename, const std::string& op) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            PrettyCLI::error("파일을 열 수 없습니다: " + filename);
            return;
        }

        std::string line;
        
        // CSV 헤더 스킵
        std::getline(file, line);
        
        // ==========================================
        // 1. 행렬곱(matmul) 처리
        // ==========================================
        if (op == "matmul") {
            std::vector<Matrix4x4> m1_list, m2_list;

            while (std::getline(file, line)) {
                if (line.empty()) continue;
                std::stringstream ss(line);
                std::string val;
                
                Matrix4x4 m1, m2;
                float* m1_ptr = reinterpret_cast<float*>(&m1);
                float* m2_ptr = reinterpret_cast<float*>(&m2);

                for (int i = 0; i < 16; ++i) {
                    if (std::getline(ss, val, ',')) *m1_ptr++ = std::stof(val);
                }
                for (int i = 0; i < 16; ++i) {
                    if (std::getline(ss, val, ',')) *m2_ptr++ = std::stof(val);
                }


                m1_list.push_back(m1);
                m2_list.push_back(m2);
            }

            size_t total_matrices = m1_list.size();
            PrettyCLI::info("총 " + std::to_string(total_matrices) + " 개의 4x4 행렬곱 연산 데이터 로드 완료.");

            // AoSoA 변환
            AoSoAMatrix4x4 m1_aosoa(total_matrices);
            AoSoAMatrix4x4 m2_aosoa(total_matrices);
            AoSoAMatrix4x4 result_aosoa(total_matrices);

            convert_aos_matrix_to_aosoa(m1_list, m1_aosoa);
            convert_aos_matrix_to_aosoa(m2_list, m2_aosoa);


            auto start = std::chrono::high_resolution_clock::now();

            simd_matrix_mul_aosoa(result_aosoa.data, m1_aosoa.data, m2_aosoa.data, total_matrices);


            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD MATRIX matmul (AoSoA)     │\n";
            std::cout << "│ 처리 행렬 수 : " << total_matrices << " 개 (총 " << total_matrices * 32 << " 개 float)       │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 행렬당 속도: " << elapsed.count() / total_matrices << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 행렬곱 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        }

        // ==========================================
        // 2. 벡터 사칙연산 (AoSoA 배치 처리)
        // ==========================================
        std::vector<Vector4> v1_list, v2_list;

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
        
        // AoSoA 변환
        AoSoAVector4 v1_aosoa(total_vectors);
        AoSoAVector4 v2_aosoa(total_vectors);
        AoSoAVector4 result_aosoa(total_vectors);

        convert_aos_to_aosoa(v1_list, v1_aosoa);
        convert_aos_to_aosoa(v2_list, v2_aosoa);

        auto start = std::chrono::high_resolution_clock::now();

        if (op == "add") {
            simd_vector_add_aosoa(result_aosoa.get_xs(), result_aosoa.get_ys(), result_aosoa.get_zs(), result_aosoa.get_ws(),
                                  v1_aosoa.get_xs(), v1_aosoa.get_ys(), v1_aosoa.get_zs(), v1_aosoa.get_ws(),
                                  v2_aosoa.get_xs(), v2_aosoa.get_ys(), v2_aosoa.get_zs(), v2_aosoa.get_ws(),
                                  total_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD VECTOR add (AoSoA)        │\n";
            std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 덧셈 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        } else if (op == "sub") {
            simd_vector_sub_aosoa(result_aosoa.get_xs(), result_aosoa.get_ys(), result_aosoa.get_zs(), result_aosoa.get_ws(),
                                  v1_aosoa.get_xs(), v1_aosoa.get_ys(), v1_aosoa.get_zs(), v1_aosoa.get_ws(),
                                  v2_aosoa.get_xs(), v2_aosoa.get_ys(), v2_aosoa.get_zs(), v2_aosoa.get_ws(),
                                  total_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD VECTOR sub (AoSoA)        │\n";
            std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 뺄셈 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        } else if (op == "mul") {
            simd_vector_mul_aosoa(result_aosoa.get_xs(), result_aosoa.get_ys(), result_aosoa.get_zs(), result_aosoa.get_ws(),
                                  v1_aosoa.get_xs(), v1_aosoa.get_ys(), v1_aosoa.get_zs(), v1_aosoa.get_ws(),
                                  v2_aosoa.get_xs(), v2_aosoa.get_ys(), v2_aosoa.get_zs(), v2_aosoa.get_ws(),
                                  total_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD VECTOR mul (AoSoA)        │\n";
            std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 곱셈 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        } else if (op == "div") {
            simd_vector_div_aosoa(result_aosoa.get_xs(), result_aosoa.get_ys(), result_aosoa.get_zs(), result_aosoa.get_ws(),
                                  v1_aosoa.get_xs(), v1_aosoa.get_ys(), v1_aosoa.get_zs(), v1_aosoa.get_ws(),
                                  v2_aosoa.get_xs(), v2_aosoa.get_ys(), v2_aosoa.get_zs(), v2_aosoa.get_ws(),
                                  total_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD VECTOR div (AoSoA)        │\n";
            std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 나눗셈 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        } else if (op == "dot") {
            std::vector<float> dot_results(total_vectors);
            simd_vector_dot_aosoa(dot_results.data(),
                                  v1_aosoa.get_xs(), v1_aosoa.get_ys(), v1_aosoa.get_zs(), v1_aosoa.get_ws(),
                                  v2_aosoa.get_xs(), v2_aosoa.get_ys(), v2_aosoa.get_zs(), v2_aosoa.get_ws(),
                                  total_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD VECTOR dot (AoSoA)        │\n";
            std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
            std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
            std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
            std::cout << "└────────────────────────────────────────────┘\n";
            std::cout << "[✔] SUCCESS: 내적 분석 프로세스가 정상 종료되었습니다.\n";
            return;
        } else if (op == "cross") {
            simd_vector_cross_aosoa(result_aosoa.get_xs(), result_aosoa.get_ys(), result_aosoa.get_zs(), result_aosoa.get_ws(),
                                    v1_aosoa.get_xs(), v1_aosoa.get_ys(), v1_aosoa.get_zs(), v1_aosoa.get_ws(),
                                    v2_aosoa.get_xs(), v2_aosoa.get_ys(), v2_aosoa.get_zs(), v2_aosoa.get_ws(),
                                    total_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed = end - start;

            std::cout << "┌─ Tesseract 하드웨어 가속 분석 보고서──────────┐\n";
            std::cout << "│ 요청된 연산 : SIMD VECTOR cross (AoSoA)     │\n";
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
        std::cout << "│ 요청된 연산 : SIMD VECTOR " << op << " (AoSoA)         │\n";
        std::cout << "│ 처리 벡터 수 : " << total_vectors << " 개 (총 " << total_vectors * 8 << " 개 float)        │\n";
        std::cout << "│ 순수 연산 시간: " << elapsed.count() << " 마이크로초 (μs)     │\n";
        std::cout << "│ 평균 벡터당 속도: " << elapsed.count() / total_vectors << " μs                      │\n";
        std::cout << "└────────────────────────────────────────────┘\n";
        std::cout << "[✔] SUCCESS: 분석 프로세스가 정상 종료되었습니다.\n";
    }
}
