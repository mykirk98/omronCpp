namespace VisionApp.Models;

public enum AppState
{
    Idle,       // 초기 상태: 자동연결 활성
    Connected,  // 연결 완료: 운전 활성
    Running,    // 운전 중: 정지 활성
    Stopped     // 정지 완료: 자동연결+운전 활성
}
