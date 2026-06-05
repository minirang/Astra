section .text
global simd_vector_add
global simd_vector_sub
global simd_vector_mul
global simd_vector_div

simd_vector_add:
    ; RDI = result, RSI = v1, RDX = v2
    movaps xmm0, [rsi]
    movaps xmm1, [rdx]
    addps xmm0, xmm1
    movaps [rdi], xmm0
    ret

simd_vector_sub:
    ; RDI = result, RSI = v1, RDX = v2
    movaps xmm0, [rsi]
    movaps xmm1, [rdx]
    subps xmm0, xmm1
    movaps [rdi], xmm0
    ret

simd_vector_mul:
    ; RDI = result, RSI = v1, RDX = v2
    movaps xmm0, [rsi]
    movaps xmm1, [rdx]
    mulps xmm0, xmm1
    movaps [rdi], xmm0
    ret

simd_vector_div:
    ; RDI = result, RSI = v1, RDX = v2
    movaps xmm0, [rsi]
    movaps xmm1, [rdx]
    divps xmm0, xmm1
    movaps [rdi], xmm0
    ret