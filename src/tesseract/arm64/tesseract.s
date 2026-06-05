.global simd_vector_add
.global simd_vector_sub
.global simd_vector_mul
.global simd_vector_div
.global simd_matrix_mul
.global simd_vector_dot
.global simd_vector_cross

.text
.align 2

simd_vector_add:
    // X0 = result, X1 = v1, X2 = v2
    ld1 {v0.4s}, [X1]
    ld1 {v1.4s}, [X2]
    fadd v2.4s, v0.4s, v1.4s
    st1 {v2.4s}, [X0]
    ret

simd_vector_sub:
    // X0 = result, X1 = v1, X2 = v2
    ld1 {v0.4s}, [X1]
    ld1 {v1.4s}, [X2]
    fsub v2.4s, v0.4s, v1.4s
    st1 {v2.4s}, [X0]
    ret

simd_vector_mul:
    // X0 = result, X1 = v1, X2 = v2
    ld1 {v0.4s}, [X1]
    ld1 {v1.4s}, [X2]
    fmul v2.4s, v0.4s, v1.4s
    st1 {v2.4s}, [X0]
    ret

simd_vector_div:
    // X0 = result, X1 = v1, X2 = v2
    ld1 {v0.4s}, [X1]
    ld1 {v1.4s}, [X2]
    fdiv v2.4s, v0.4s, v1.4s
    st1 {v2.4s}, [X0]
    ret

simd_matrix_mul:
    // X0 = result, X1 = A, X2 = B, X3

    // 뒤 행렬(m2)의 모든 데이터(행 4개)를 레지스터 v16 ~ v19에 미리 로드해놓는다.
    ld1 {v16.4s}, [X2], #16
    ld1 {v17.4s}, [X2], #16
    ld1 {v18.4s}, [X2], #16
    ld1 {v19.4s}, [X2], #16

    // ==========================================
    // [결과 행렬의 1번째 행 계산]
    // ==========================================
    ld1 {v0.4s}, [X1], #16  // m1의 1번째 행을 v0에 로드

    // v0의 x, y, z, w 성분을 쪼개서 m2의 행들과 곱하고 더함 (fmla 키워드 사용!)
    fmul v20.4s, v16.4s, v0.s[0]       // m2의 1행 * m1.row[0].x
    fmla v20.4s, v17.4s, v0.s[1]       // + m2의 2행 * m1.row[0].y
    fmla v20.4s, v18.4s, v0.s[2]       // + m2의 3행 * m1.row[0].z
    fmla v20.4s, v19.4s, v0.s[3]       // + m2의 4행 * m1.row[0].w

    st1 {v20.4s}, [X0], #16 // 결과를 결과 행렬의 1번째 행에 저장

    // ==========================================
    // [결과 행렬의 2번째 행 계산]
    // ==========================================

    ld1 {v0.4s}, [X1], #16  // m1의 2번째 행을 v0에 로드

    fmul v21.4s, v16.4s, v1.s[0]       // m2의 1행 * m1.row[1].x
    fmla v21.4s, v17.4s, v1.s[1]       // + m2의 2행 * m1.row[1].y
    fmla v21.4s, v18.4s, v1.s[2]       // + m2의 3행 * m1.row[1].z
    fmla v21.4s, v19.4s, v1.s[3]       // + m2의 4행 * m1.row[1].w

    st1 {v21.4s}, [X0], #16 // 결과를 결과 행렬의 2번째 행에 저장

    // ==========================================
    // [결과 행렬의 3번째 행 계산]
    // ==========================================
    ld1 {v2.4s}, [X1], #16  // m1의 3번째 행을 v2에 로드

    fmul v22.4s, v16.4s, v2.s[0]
    fmla v22.4s, v17.4s, v2.s[1]
    fmla v22.4s, v18.4s, v2.s[2]
    fmla v22.4s, v19.4s, v2.s[3]

    st1 {v22.4s}, [X0], #16 // 결과 저장

    // ==========================================
    // [결과 행렬의 4번째 행 계산]
    // ==========================================
    ld1 {v3.4s}, [X1]       // m1의 4번째 행을 v3에 로드

    fmul v23.4s, v16.4s, v3.s[0]
    fmla v23.4s, v17.4s, v3.s[1]
    fmla v23.4s, v18.4s, v3.s[2]
    fmla v23.4s, v19.4s, v3.s[3]

    st1 {v23.4s}, [X0]      // 마지막 결과 저장
    ret

simd_vector_dot:
    // X0 = 결과 float 주소, X1 = v1 주소, X2 = v2 주소
    ld1 {v0.4s}, [X1]           // v1 로드
    ld1 {v1.4s}, [X2]           // v2 로드
    
    fmul v2.4s, v0.4s, v1.4s    // 1:1로 곱하기 (x1*x2, y1*y2, z1*z2, w1*w2)
    
    // 흩어진 4개의 결과를 더해서 압축하기
    faddp v3.4s, v2.4s, v2.4s   // [x+y, z+w, x+y, z+w] 형태로 페어 더하기
    faddp v4.4s, v3.4s, v3.4s   // 최종적으로 다 더해진 값이 v4.s[0]에 모임
    
    str s4, [X0]                // 결과 float를 메모리에 저장
    ret

simd_vector_cross:
    // X0 = 결과 벡터 주소, X1 = v1 주소, X2 = v2 주소
    ld1 {v0.4s}, [X1]           // v1 로드 [x1, y1, z1, w1]
    ld1 {v1.4s}, [X2]           // v2 로드 [x2, y2, z2, w2]

    // 1. 데이터를 지그재그로 회전시켜서 크로스할 준비를 함 (ext 사용)
    // v2 = [y1, z1, x1, w1] 형태로 회전
    ext v2.16b, v0.16b, v0.16b, #4
    // v3 = [z2, x2, y2, w2] 형태로 회전
    ext v3.16b, v1.16b, v1.16b, #8
    
    // 2. 첫 번째 크로스 곱셈 수행 (A.y * B.z 등등)
    fmul v4.4s, v2.4s, v3.4s

    // 3. 반대 방향으로 크로스할 준비
    // v5 = [z1, x1, y1, w1] 형태로 회전
    ext v5.16b, v0.16b, v0.16b, #8
    // v6 = [y2, z2, x2, w2] 형태로 회전
    ext v6.16b, v1.16b, v1.16b, #4

    // 4. 두 번째 크로스 곱셈을 한 뒤, 기존 결과에서 빼버리기 (fmls = 곱해서 빼라!)
    fmls v4.4s, v5.4s, v6.4s

    // 5. 3D 공간 벡터이므로 마지막 W 성분은 깔끔하게 0으로 리셋
    mov v4.s[3], wzr

    st1 {v4.4s}, [X0]           // 결과 저장
    ret