#ifndef TESSERACT_HPP
#define TESSERACT_HPP

#include <string>
#include <vector>
#include <cstring>

namespace Tesseract {
    // 16바이트 정렬된 4D 벡터 구조체
    struct alignas(16) Vector4 {
        float x, y, z, w;
    };

    // 4x4 행렬 구조체
    struct alignas(16) Matrix4x4 {
        Vector4 rows[4];
    };

    // ============================================
    // AoSoA (Array of Structures of Arrays)
    // 배치 크기: 8 (AVX2 = 256비트 / 32비트 = 8개 float)
    // 정렬: 64바이트 (L1 캐시라인)
    // 구조: [batch0: xs[8], ys[8], zs[8], ws[8]], [batch1: ...], ...
    // ============================================
    
    struct alignas(64) AoSoAVector4 {
        static constexpr size_t BATCH_SIZE = 8;
        
        float* data;        // 단일 연속 메모리 블록 (배치별로 xs, ys, zs, ws 저장)
        size_t batch_count; // 배치 개수
        size_t total_count; // 실제 벡터 개수
        
        AoSoAVector4(size_t size) {
            total_count = size;
            batch_count = (size + BATCH_SIZE - 1) / BATCH_SIZE; // 올림
            size_t alloc_size = batch_count * BATCH_SIZE * 4; // xs, ys, zs, ws
            data = new float[alloc_size]();
        }
        
        ~AoSoAVector4() {
            delete[] data;
        }
        
        // zero-copy: 포인터만 반환
        inline float* get_xs() const { return data; }
        inline float* get_ys() const { return data + batch_count * BATCH_SIZE; }
        inline float* get_zs() const { return data + batch_count * BATCH_SIZE * 2; }
        inline float* get_ws() const { return data + batch_count * BATCH_SIZE * 3; }
        
        // 복사 생성 방지 (zero-copy)
        AoSoAVector4(const AoSoAVector4&) = delete;
        AoSoAVector4& operator=(const AoSoAVector4&) = delete;
    };

    struct alignas(64) AoSoAMatrix4x4 {
        static constexpr size_t BATCH_SIZE = 8;
        
        float* data;        // 단일 연속 메모리: 16 rows × batch_count × BATCH_SIZE
        size_t batch_count;
        size_t total_count;
        
        AoSoAMatrix4x4(size_t size) {
            total_count = size;
            batch_count = (size + BATCH_SIZE - 1) / BATCH_SIZE;
            size_t alloc_size = batch_count * BATCH_SIZE * 16; // 4x4 = 16 float
            data = new float[alloc_size]();
        }
        
        ~AoSoAMatrix4x4() {
            delete[] data;
        }
        
        // 행렬 배치 접근 (zero-copy 포인터)
        inline float* get_row(int row) const {
            return data + row * batch_count * BATCH_SIZE;
        }
        
        AoSoAMatrix4x4(const AoSoAMatrix4x4&) = delete;
        AoSoAMatrix4x4& operator=(const AoSoAMatrix4x4&) = delete;
    };

    extern "C" {
        // ========================================
        // AoSoA 배치 처리 SIMD 함수들 (극한 최적화)
        // ========================================
        
        // 벡터 사칙연산 (배치 단위, zero-copy)
        void simd_vector_add_aosoa(float* result_xs, float* result_ys, float* result_zs, float* result_ws,
                                   const float* v1_xs, const float* v1_ys, const float* v1_zs, const float* v1_ws,
                                   const float* v2_xs, const float* v2_ys, const float* v2_zs, const float* v2_ws,
                                   size_t total_count);
        void simd_vector_sub_aosoa(float* result_xs, float* result_ys, float* result_zs, float* result_ws,
                                   const float* v1_xs, const float* v1_ys, const float* v1_zs, const float* v1_ws,
                                   const float* v2_xs, const float* v2_ys, const float* v2_zs, const float* v2_ws,
                                   size_t total_count);
        void simd_vector_mul_aosoa(float* result_xs, float* result_ys, float* result_zs, float* result_ws,
                                   const float* v1_xs, const float* v1_ys, const float* v1_zs, const float* v1_ws,
                                   const float* v2_xs, const float* v2_ys, const float* v2_zs, const float* v2_ws,
                                   size_t total_count);
        void simd_vector_div_aosoa(float* result_xs, float* result_ys, float* result_zs, float* result_ws,
                                   const float* v1_xs, const float* v1_ys, const float* v1_zs, const float* v1_ws,
                                   const float* v2_xs, const float* v2_ys, const float* v2_zs, const float* v2_ws,
                                   size_t total_count);
        
        // 벡터 고급 연산 (배치 단위)
        void simd_vector_dot_aosoa(float* results,
                                   const float* v1_xs, const float* v1_ys, const float* v1_zs, const float* v1_ws,
                                   const float* v2_xs, const float* v2_ys, const float* v2_zs, const float* v2_ws,
                                   size_t total_count);
        void simd_vector_cross_aosoa(float* result_xs, float* result_ys, float* result_zs, float* result_ws,
                                     const float* v1_xs, const float* v1_ys, const float* v1_zs, const float* v1_ws,
                                     const float* v2_xs, const float* v2_ys, const float* v2_zs, const float* v2_ws,
                                     size_t total_count);
        
        // 행렬 연산 (배치 단위)
        void simd_matrix_mul_aosoa(float* result,
                                   const float* m1_rows, const float* m2_rows,
                                   size_t total_count);
    }

    // CSV 파싱 및 실행 제어 함수
    void run_simd_analyzer(const std::string& filename, const std::string& op);
}

#endif