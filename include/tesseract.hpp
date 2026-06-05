#ifndef TESSERACT_HPP
#define TESSERACT_HPP

#include <string>
#include <vector>

namespace Tesseract {
    // 16바이트 정렬된 4D 벡터 구조체
    struct alignas(16) Vector4 {
        float x, y, z, w;
    };

    //4x4 행렬 구조체
    struct alignas(16) Matrix4x4 {
        Vector4 rows[4];
    };

    extern "C" {
        // OS별 어셈블리에서 구현할 SIMD 가속 함수들
        void simd_vector_add(Vector4* result, const Vector4* v1, const Vector4* v2);
        void simd_vector_sub(Vector4* result, const Vector4* v1, const Vector4* v2);
        void simd_vector_mul(Vector4* result, const Vector4* v1, const Vector4* v2);
        void simd_vector_div(Vector4* result, const Vector4* v1, const Vector4* v2);
        void simd_matrix_mul(Matrix4x4* result, const Matrix4x4* m1, const Matrix4x4* m2);
        void simd_vector_dot(float* result, const Vector4* v1, const Vector4* v2);
        void simd_vector_cross(Vector4* result, const Vector4* v1, const Vector4* v2);
    }

    // CSV 파싱 및 실행 제어 함수
    void run_simd_analyzer(const std::string& filename, const std::string& op);
}

#endif