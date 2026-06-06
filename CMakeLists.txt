cmake_minimum_required(VERSION 4.2)

# 1. 처음엔 CXX(C++)만 활성화해서 프로젝트 시작 (예외 방지)
project(Astra CXX) 

# C++23 표준 강제 설정
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 헤더 파일 참조 경로 등록
include_directories(
    .
    bluskpm
    include
    src
    web
)

# 모든 운영체제 공통 소스 파일 목록
set(COMMON_SOURCES
    main.cpp
    src/coodbooks/openwindow.cpp
    src/exodus/continueloops.cpp
    src/prettyCLI/prettycli.cpp
    src/ramon/ramon.cpp
    src/ramon/ramoptimum.c
    src/tesseract/tesseract.cpp
    src/tesseract/3Drenderer.cpp
)

# 2. [핵심] 아키텍처에 맞는 어셈블러만 동적으로 활성화 (enable_language)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CMAKE_SYSTEM_PROCESSOR_LOWER)
message(STATUS "호스트 아키텍처 검사: ${CMAKE_SYSTEM_PROCESSOR} -> ${CMAKE_SYSTEM_PROCESSOR_LOWER}")

if(CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "amd64" OR CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "x64")
    # Intel / AMD 환경일 때만 NASM 컴파일러를 활성화함
    find_program(NASM_EXECUTABLE nasm)
    if(NOT NASM_EXECUTABLE)
        message(FATAL_ERROR "NASM 어셈블러를 찾을 수 없습니다. Linux에서는 'sudo apt install nasm' 또는 패키지 매니저로 nasm을 설치하세요.")
    endif()
    set(CMAKE_ASM_NASM_COMPILER "${NASM_EXECUTABLE}")
    enable_language(ASM_NASM)

    # 공통 소스에 들어있는 x86-64용 어셈블리 추가
    list(APPEND COMMON_SOURCES 
        src/exodus/continueloops.asm 
        src/ramon/ramoptimum.asm
    )

    if(WIN32)
        set(OS_SPECIFIC_ASM 
            src/tesseract/win64/tesseract.asm
            src/tesseract/win64/3Drenderer.asm
        )
    else()
        set(OS_SPECIFIC_ASM 
            src/tesseract/linux/tesseract.asm
            src/tesseract/linux/3Drenderer.asm
        )
    endif()

elseif(CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "arm64" OR CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "armv8" OR CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "armv8l")
    # ★ 라즈베리파이(ARM64) 환경일 때는 일반 ASM(GNU Assembler)만 활성화!
    enable_language(ASM)
    
    # [참고] 만약 continueloops나 ramoptimum도 x86 용 asm 파일이라면 
    # 라즈베리파이 빌드 시에는 공통에서 제외해야 에러가 안 나! 
    # (여기서는 일단 제외하거나 추후 arm64용으로 대체해야 함)

    set(OS_SPECIFIC_ASM 
        src/tesseract/arm64/tesseract.s
        src/tesseract/arm64/3Drenderer.s
    )
else()
    message(FATAL_ERROR "Astra 엔진이 지원하지 않는 하드웨어 아키텍처입니다: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# 3. 최종 빌드 실행 파일 생성
add_executable(Astra
    ${COMMON_SOURCES}
    ${OS_SPECIFIC_ASM}
)