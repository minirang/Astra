#include "include/prettycli.hpp"
#include "include/tesseract.hpp"
#include <chrono>
#include <thread>

int main(int argc, char* argv[]) {
    PrettyCLI::initTerminal();
    PrettyCLI::drawLogo();

    /*
    PrettyCLI::info("Astra 패키지 다운로드 중...");

    // 예시: 실제 환경에서는 다운로드한 바이트 수나 파일 청크 수(chunk)가 될 수 있어.
    long long totalBytes = 52428800; // 50MB 짜리 패키지라고 가정
    long long downloadedBytes = 0;
    long long chunkSize = 51200;    // 한 번에 50KB씩 읽어옴 (루프가 총 1024번 돎)

    // ★ 핵심: 이전 퍼센트를 기억할 변수 (-1로 초기화)
    int lastPercent = -1;

    while (downloadedBytes < totalBytes) {
        // 다운로드 시뮬레이션 (실제로는 여기서 네트워크나 파일 I/O가 일어남)
        downloadedBytes += chunkSize;
        if (downloadedBytes > totalBytes) downloadedBytes = totalBytes;

        // 진행률 계산 (0.0 ~ 1.0)
        double progress = static_cast<double>(downloadedBytes) / totalBytes;

        // 현재 정수 퍼센트 계산 (0 ~ 100)
        int currentPercent = static_cast<int>(progress * 100.0);

        // ★ [조건문] 퍼센트가 실제로 변했을 때만 화면을 새로 그린다!
        // 이 조건 덕분에 루프가 10만 번을 돌든 100만 번을 돌든, drawProgressBar는 딱 100번만 실행됨.
        if (currentPercent != lastPercent) {
            PrettyCLI::drawProgressBar("bluskpm/socket", progress);
            lastPercent = currentPercent; // 최신 퍼센트로 업데이트
        }

        // CPU 과점 방지 및 시뮬레이션을 위한 미세한 딜레이
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::cout << "\n"; // 진행 바 끝나면 줄 바꿈
    PrettyCLI::success("패키지 설치가 완료되었습니다.");

    */

    PrettyCLI::initTerminal();

    // 사용법 안내: simd <파일이름> <연산자>
    if (argc < 4 || std::string(argv[1]) != "simd") {
        PrettyCLI::drawLogo();
        std::cout << "사용법: .\\Astra.exe simd <파일경로.csv> <add|sub>\n";
        return 1;
    }

    std::string filename = argv[2];
    std::string operation = argv[3];

    Tesseract::run_simd_analyzer(filename, operation);

    return 0;
}