section .text
global simd_vector_add
global simd_vector_sub
global simd_vector_mul
global simd_vector_div

simd_vector_add:
    ; RCX = result, RDX = v1, R8 = v2
    movaps xmm0, [rdx]
    movaps xmm1, [r8]
    addps xmm0, xmm1
    movaps [rcx], xmm0
    ret

simd_vector_sub:
    ; RCX = result, RDX = v1, R8 = v2
    movaps xmm0, [rdx]
    movaps xmm1, [r8]
    subps xmm0, xmm1
    movaps [rcx], xmm0
    ret

simd_vector_mul:
    ; RCX = result, RDX = v1, R8 = v2
    movaps xmm0, [rdx]
    movaps xmm1, [r8]
    mulps xmm0, xmm1
    movaps [rcx], xmm0
    ret

simd_vector_div:
    ; RCX = result, RDX = v1, R8 = v2
    movaps xmm0, [rdx]
    movaps xmm1, [r8]
    divps xmm0, xmm1
    movaps [rcx], xmm0
    ret