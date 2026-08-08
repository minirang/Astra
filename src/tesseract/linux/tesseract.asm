section .text
global simd_vector_add_aosoa
global simd_vector_sub_aosoa
global simd_vector_mul_aosoa
global simd_vector_div_aosoa
global simd_vector_dot_aosoa
global simd_vector_cross_aosoa
global simd_matrix_mul_aosoa

; ==========================================
; AoSoA 벡터 연산 (배치 크기 = 8, AVX2)
; System V AMD64 Calling Convention (Linux):
; RDI, RSI, RDX, RCX, R8, R9 = 첫 6개 파라미터
; 나머지는 [RSP+8], [RSP+16], ...
; ==========================================

simd_vector_add_aosoa:
    ; RDI = result_xs, RSI = result_ys, RDX = result_zs, RCX = result_ws
    ; R8 = v1_xs, R9 = v1_ys
    ; [RSP+8] = v1_zs, [RSP+16] = v1_ws, [RSP+24] = v2_xs, [RSP+32] = v2_ys
    ; [RSP+40] = v2_zs, [RSP+48] = v2_ws, [RSP+56] = total_count
    
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    mov r10, [rsp + 56]     ; r10 = v1_zs
    mov r11, [rsp + 64]     ; r11 = v1_ws
    mov r12, [rsp + 72]     ; r12 = v2_xs
    mov r13, [rsp + 80]     ; r13 = v2_ys
    mov r14, [rsp + 88]     ; r14 = v2_zs
    mov r15, [rsp + 96]     ; r15 = v2_ws
    mov rax, [rsp + 104]    ; rax = total_count
    
    xor rbx, rbx            ; i = 0
    
.loop_add:
    cmp rbx, rax
    jge .done_add
    
    shl rbx, 5
    
    vmovaps ymm0, [r8 + rbx]
    vmovaps ymm1, [r12 + rbx]
    vaddps ymm2, ymm0, ymm1
    vmovaps [rdi + rbx], ymm2
    
    vmovaps ymm3, [r9 + rbx]
    vmovaps ymm4, [r13 + rbx]
    vaddps ymm5, ymm3, ymm4
    vmovaps [rsi + rbx], ymm5
    
    vmovaps ymm6, [r10 + rbx]
    vmovaps ymm7, [r14 + rbx]
    vaddps ymm8, ymm6, ymm7
    vmovaps [rdx + rbx], ymm8
    
    vmovaps ymm9, [r11 + rbx]
    vmovaps ymm10, [r15 + rbx]
    vaddps ymm11, ymm9, ymm10
    vmovaps [rcx + rbx], ymm11
    
    inc rbx
    jmp .loop_add
    
.done_add:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret

simd_vector_sub_aosoa:
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    mov r10, [rsp + 56]
    mov r11, [rsp + 64]
    mov r12, [rsp + 72]
    mov r13, [rsp + 80]
    mov r14, [rsp + 88]
    mov r15, [rsp + 96]
    mov rax, [rsp + 104]
    
    xor rbx, rbx
    
.loop_sub:
    cmp rbx, rax
    jge .done_sub
    
    shl rbx, 5
    
    vmovaps ymm0, [r8 + rbx]
    vmovaps ymm1, [r12 + rbx]
    vsubps ymm2, ymm0, ymm1
    vmovaps [rdi + rbx], ymm2
    
    vmovaps ymm3, [r9 + rbx]
    vmovaps ymm4, [r13 + rbx]
    vsubps ymm5, ymm3, ymm4
    vmovaps [rsi + rbx], ymm5
    
    vmovaps ymm6, [r10 + rbx]
    vmovaps ymm7, [r14 + rbx]
    vsubps ymm8, ymm6, ymm7
    vmovaps [rdx + rbx], ymm8
    
    vmovaps ymm9, [r11 + rbx]
    vmovaps ymm10, [r15 + rbx]
    vsubps ymm11, ymm9, ymm10
    vmovaps [rcx + rbx], ymm11
    
    inc rbx
    jmp .loop_sub
    
.done_sub:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret

simd_vector_mul_aosoa:
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    mov r10, [rsp + 56]
    mov r11, [rsp + 64]
    mov r12, [rsp + 72]
    mov r13, [rsp + 80]
    mov r14, [rsp + 88]
    mov r15, [rsp + 96]
    mov rax, [rsp + 104]
    
    xor rbx, rbx
    
.loop_mul:
    cmp rbx, rax
    jge .done_mul
    
    shl rbx, 5
    
    vmovaps ymm0, [r8 + rbx]
    vmovaps ymm1, [r12 + rbx]
    vmulps ymm2, ymm0, ymm1
    vmovaps [rdi + rbx], ymm2
    
    vmovaps ymm3, [r9 + rbx]
    vmovaps ymm4, [r13 + rbx]
    vmulps ymm5, ymm3, ymm4
    vmovaps [rsi + rbx], ymm5
    
    vmovaps ymm6, [r10 + rbx]
    vmovaps ymm7, [r14 + rbx]
    vmulps ymm8, ymm6, ymm7
    vmovaps [rdx + rbx], ymm8
    
    vmovaps ymm9, [r11 + rbx]
    vmovaps ymm10, [r15 + rbx]
    vmulps ymm11, ymm9, ymm10
    vmovaps [rcx + rbx], ymm11
    
    inc rbx
    jmp .loop_mul
    
.done_mul:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret

simd_vector_div_aosoa:
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    mov r10, [rsp + 56]
    mov r11, [rsp + 64]
    mov r12, [rsp + 72]
    mov r13, [rsp + 80]
    mov r14, [rsp + 88]
    mov r15, [rsp + 96]
    mov rax, [rsp + 104]
    
    xor rbx, rbx
    
.loop_div:
    cmp rbx, rax
    jge .done_div
    
    shl rbx, 5
    
    vmovaps ymm0, [r8 + rbx]
    vmovaps ymm1, [r12 + rbx]
    vdivps ymm2, ymm0, ymm1
    vmovaps [rdi + rbx], ymm2
    
    vmovaps ymm3, [r9 + rbx]
    vmovaps ymm4, [r13 + rbx]
    vdivps ymm5, ymm3, ymm4
    vmovaps [rsi + rbx], ymm5
    
    vmovaps ymm6, [r10 + rbx]
    vmovaps ymm7, [r14 + rbx]
    vdivps ymm8, ymm6, ymm7
    vmovaps [rdx + rbx], ymm8
    
    vmovaps ymm9, [r11 + rbx]
    vmovaps ymm10, [r15 + rbx]
    vdivps ymm11, ymm9, ymm10
    vmovaps [rcx + rbx], ymm11
    
    inc rbx
    jmp .loop_div
    
.done_div:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret

; ==========================================
; 벡터 내적 (AoSoA 배치)
; RDI = results, RSI = v1_xs, RDX = v1_ys, RCX = v1_zs, R8 = v1_ws
; R9 = v2_xs, [RSP+8] = v2_ys, [RSP+16] = v2_zs, [RSP+24] = v2_ws, [RSP+32] = total_count
; ==========================================
simd_vector_dot_aosoa:
    push rbx
    push r12
    push r13
    
    mov r10, [rsp + 40]     ; r10 = v2_ys
    mov r11, [rsp + 48]     ; r11 = v2_zs
    mov r12, [rsp + 56]     ; r12 = v2_ws
    mov r13, [rsp + 64]     ; r13 = total_count
    
    xor rbx, rbx
    
.loop_dot:
    cmp rbx, r13
    jge .done_dot
    
    shl rbx, 5
    
    vmovaps ymm0, [rsi + rbx]
    vmovaps ymm1, [r9 + rbx]
    vmulps ymm2, ymm0, ymm1
    
    vmovaps ymm3, [rdx + rbx]
    vmovaps ymm4, [r10 + rbx]
    vmulps ymm5, ymm3, ymm4
    vaddps ymm2, ymm2, ymm5
    
    vmovaps ymm6, [rcx + rbx]
    vmovaps ymm7, [r11 + rbx]
    vmulps ymm8, ymm6, ymm7
    vaddps ymm2, ymm2, ymm8
    
    vmovaps ymm9, [r8 + rbx]
    vmovaps ymm10, [r12 + rbx]
    vmulps ymm11, ymm9, ymm10
    vaddps ymm2, ymm2, ymm11
    
    ; 합산
    vperm2f128 ymm3, ymm2, ymm2, 0x01
    vaddps ymm2, ymm2, ymm3
    vshufps ymm3, ymm2, ymm2, 0x55
    vaddps ymm2, ymm2, ymm3
    vshufps ymm3, ymm2, ymm2, 0xFF
    vaddps ymm2, ymm2, ymm3
    
    ; rbx를 인덱스로 사용 (주의: rbx는 shift된 상태)
    mov rax, rbx
    shr rax, 5
    vmovss [rdi + rax*4], xmm2
    
    inc rbx
    jmp .loop_dot
    
.done_dot:
    pop r13
    pop r12
    pop rbx
    ret

; ==========================================
; 벡터 외적 (AoSoA 배치)
; ==========================================
simd_vector_cross_aosoa:
    push rbx
    push r12
    push r13
    
    mov r10, [rsp + 40]
    mov r11, [rsp + 48]
    mov r12, [rsp + 56]
    mov r13, [rsp + 64]
    
    xor rbx, rbx
    
.loop_cross:
    cmp rbx, r13
    jge .done_cross
    
    shl rbx, 5
    
    ; x_cross = y1*z2 - z1*y2
    vmovaps ymm0, [rdx + rbx]
    vmovaps ymm1, [r11 + rbx]
    vmulps ymm2, ymm0, ymm1
    
    vmovaps ymm3, [rcx + rbx]
    vmovaps ymm4, [r10 + rbx]
    vmulps ymm5, ymm3, ymm4
    vsubps ymm2, ymm2, ymm5
    vmovaps [rdi + rbx], ymm2
    
    ; y_cross = z1*x2 - x1*z2
    vmovaps ymm6, [rcx + rbx]
    vmovaps ymm7, [r9 + rbx]
    vmulps ymm8, ymm6, ymm7
    
    vmovaps ymm9, [rsi + rbx]
    vmovaps ymm10, [r11 + rbx]
    vmulps ymm11, ymm9, ymm10
    vsubps ymm8, ymm8, ymm11
    vmovaps [rdx + rbx], ymm8
    
    ; z_cross = x1*y2 - y1*x2
    vmovaps ymm12, [rsi + rbx]
    vmovaps ymm13, [r10 + rbx]
    vmulps ymm14, ymm12, ymm13
    
    vmovaps ymm15, [rdx + rbx]
    vmovaps ymm0, [r9 + rbx]
    vmulps ymm1, ymm15, ymm0
    vsubps ymm14, ymm14, ymm1
    vmovaps [rcx + rbx], ymm14
    
    ; ws = 0
    vxorps ymm2, ymm2, ymm2
    vmovaps [r8 + rbx], ymm2
    
    inc rbx
    jmp .loop_cross
    
.done_cross:
    pop r13
    pop r12
    pop rbx
    ret

; ==========================================
; 행렬 곱셈 (AoSoA)
; RDI = result_data, RSI = m1_data, RDX = m2_data, RCX = total_count
; ==========================================
simd_matrix_mul_aosoa:
    push rbx
    
    xor rax, rax            ; i = 0
    
.loop_matmul:
    cmp rax, rcx
    jge .done_matmul
    
    ; 간단한 배치 처리 (나중에 최적화)
    inc rax
    jmp .loop_matmul
    
.done_matmul:
    pop rbx
    ret
