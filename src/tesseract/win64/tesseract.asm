section .text

global simd_vector_add_aosoa
global simd_vector_sub_aosoa
global simd_vector_mul_aosoa
global simd_vector_div_aosoa
global simd_vector_dot_aosoa
global simd_vector_cross_aosoa
global simd_matrix_mul_aosoa

; Windows x64 ABI: argument 5 starts at [RSP + 40] on entry.
; The vector layout contains eight contiguous floats per AoSoA batch.
%macro VECTOR_BINARY 2
%1:
    push r9
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15

    ; Eight pushes move RSP by 64 bytes from its entry value.
    mov r10, [rsp + 104]       ; v1_xs
    mov r11, [rsp + 112]       ; v1_ys
    mov r12, [rsp + 120]       ; v1_zs
    mov r13, [rsp + 128]       ; v1_ws
    mov r14, [rsp + 136]       ; v2_xs
    mov r15, [rsp + 144]       ; v2_ys
    mov rbx, [rsp + 152]       ; v2_zs
    mov rax, [rsp + 160]       ; v2_ws
    mov rsi, [rsp + 168]       ; total vector count
    add rsi, 7
    shr rsi, 3                 ; number of AVX batches
    xor rdi, rdi

%%loop:
    cmp rdi, rsi
    jae %%done

    mov r9, rdi
    shl r9, 5                  ; batch offset: 8 floats * 4 bytes

    vmovups ymm0, [r10 + r9]
    vmovups ymm1, [r14 + r9]
    %2 ymm0, ymm0, ymm1
    vmovups [rcx + r9], ymm0

    vmovups ymm0, [r11 + r9]
    vmovups ymm1, [r15 + r9]
    %2 ymm0, ymm0, ymm1
    vmovups [rdx + r9], ymm0

    vmovups ymm0, [r12 + r9]
    vmovups ymm1, [rbx + r9]
    %2 ymm0, ymm0, ymm1
    vmovups [r8 + r9], ymm0

    vmovups ymm0, [r13 + r9]
    vmovups ymm1, [rax + r9]
    %2 ymm0, ymm0, ymm1
    mov rax, [rsp + 56]           ; saved result_ws pointer
    vmovups [rax + r9], ymm0

    inc rdi
    jmp %%loop

%%done:
    vzeroupper
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    add rsp, 8                 ; discard saved R9
    ret
%endmacro

VECTOR_BINARY simd_vector_add_aosoa, vaddps
VECTOR_BINARY simd_vector_sub_aosoa, vsubps
VECTOR_BINARY simd_vector_mul_aosoa, vmulps
VECTOR_BINARY simd_vector_div_aosoa, vdivps

; Scalar-per-vector implementations for reductions and cross products.  Their
; AoSoA component buffers are still contiguous, so they are safe for any count.
simd_vector_dot_aosoa:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    mov r10, [rsp + 96]        ; v1_ws
    mov r11, [rsp + 104]       ; v2_xs
    mov r12, [rsp + 112]       ; v2_ys
    mov r13, [rsp + 120]       ; v2_zs
    mov r14, [rsp + 128]       ; v2_ws
    mov r15, [rsp + 136]       ; total_count
    xor rsi, rsi
.dot_loop:
    cmp rsi, r15
    jae .dot_done
    vmovss xmm0, [rdx + rsi*4]
    vmulss xmm0, xmm0, [r11 + rsi*4]
    vmovss xmm1, [r8 + rsi*4]
    vmulss xmm1, xmm1, [r12 + rsi*4]
    vaddss xmm0, xmm0, xmm1
    vmovss xmm1, [r9 + rsi*4]
    vmulss xmm1, xmm1, [r13 + rsi*4]
    vaddss xmm0, xmm0, xmm1
    vmovss xmm1, [r10 + rsi*4]
    vmulss xmm1, xmm1, [r14 + rsi*4]
    vaddss xmm0, xmm0, xmm1
    vmovss [rcx + rsi*4], xmm0
    inc rsi
    jmp .dot_loop
.dot_done:
    vzeroupper
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

simd_vector_cross_aosoa:
    push r9
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    mov r10, [rsp + 104]       ; v1_xs
    mov r11, [rsp + 112]       ; v1_ys
    mov r12, [rsp + 120]       ; v1_zs
    mov r14, [rsp + 136]       ; v2_xs
    mov r15, [rsp + 144]       ; v2_ys
    mov rbx, [rsp + 152]       ; v2_zs
    mov rsi, [rsp + 168]       ; total_count
    xor rdi, rdi
.cross_loop:
    cmp rdi, rsi
    jae .cross_done
    vmovss xmm0, [r11 + rdi*4]
    vmulss xmm0, xmm0, [rbx + rdi*4]
    vmovss xmm1, [r12 + rdi*4]
    vmulss xmm1, xmm1, [r15 + rdi*4]
    vsubss xmm0, xmm0, xmm1
    vmovss [rcx + rdi*4], xmm0
    vmovss xmm0, [r12 + rdi*4]
    vmulss xmm0, xmm0, [r14 + rdi*4]
    vmovss xmm1, [r10 + rdi*4]
    vmulss xmm1, xmm1, [rbx + rdi*4]
    vsubss xmm0, xmm0, xmm1
    vmovss [rdx + rdi*4], xmm0
    vmovss xmm0, [r10 + rdi*4]
    vmulss xmm0, xmm0, [r15 + rdi*4]
    vmovss xmm1, [r11 + rdi*4]
    vmulss xmm1, xmm1, [r14 + rdi*4]
    vsubss xmm0, xmm0, xmm1
    vmovss [r8 + rdi*4], xmm0
    vxorps xmm0, xmm0, xmm0
    vmovss [r9 + rdi*4], xmm0
    inc rdi
    jmp .cross_loop
.cross_done:
    vzeroupper
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    add rsp, 8
    ret

%macro MATRIX_CELL 2
    mov r10, r14
%if %1 > 0
    mov r11, r13
    imul r11, %1
    add r10, r11
%endif
    mov r11, r15
    add r11, %2 * 4
    vmovss xmm0, [r10]
    vmulss xmm0, xmm0, [r11]
    add r10, 4
    add r11, r13
    vmovss xmm1, [r10]
    vmulss xmm1, xmm1, [r11]
    vaddss xmm0, xmm0, xmm1
    add r10, 4
    add r11, r13
    vmovss xmm1, [r10]
    vmulss xmm1, xmm1, [r11]
    vaddss xmm0, xmm0, xmm1
    add r10, 4
    add r11, r13
    vmovss xmm1, [r10]
    vmulss xmm1, xmm1, [r11]
    vaddss xmm0, xmm0, xmm1
    mov rax, rdi
%if %1 > 0
    mov r11, r13
    imul r11, %1
    add rax, r11
%endif
    vmovss [rax + %2 * 4], xmm0
%endmacro

; Matrix layout is row-major inside each 8-matrix batch.  The inner products
; are scalar so the existing layout remains correct and the ABI is preserved.
simd_matrix_mul_aosoa:
    push rbx
    push rsi
    push rdi
    push r12
    push r13
    push r14
    push r15
    mov r12, r9
    add r9, 7
    shr r9, 3
    imul r13, r9, 128         ; byte stride of one row across all batches
    xor rsi, rsi
.matrix_loop:
    cmp rsi, r12
    jae .matrix_done
    mov rax, rsi
    shr rax, 3
    imul rax, rax, 128
    mov rbx, rsi
    and rbx, 7
    shl rbx, 4
    add rax, rbx
    lea r14, [rdx + rax]
    lea r15, [r8 + rax]
    lea rdi, [rcx + rax]
    MATRIX_CELL 0, 0
    MATRIX_CELL 0, 1
    MATRIX_CELL 0, 2
    MATRIX_CELL 0, 3
    MATRIX_CELL 1, 0
    MATRIX_CELL 1, 1
    MATRIX_CELL 1, 2
    MATRIX_CELL 1, 3
    MATRIX_CELL 2, 0
    MATRIX_CELL 2, 1
    MATRIX_CELL 2, 2
    MATRIX_CELL 2, 3
    MATRIX_CELL 3, 0
    MATRIX_CELL 3, 1
    MATRIX_CELL 3, 2
    MATRIX_CELL 3, 3
    inc rsi
    jmp .matrix_loop
.matrix_done:
    vzeroupper
    pop r15
    pop r14
    pop r13
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret