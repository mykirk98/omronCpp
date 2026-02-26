namespace VisionApp.Models;

// 툴바 버튼들의 활성/비활성 상태를 제어하는 상태 머신
public enum AppState
{
    Idle,       // 초기 상태: 자동연결 활성
    Connected,  // 연결 완료: 운전 활성
    Running,    // 운전 중: 정지 활성
    Stopped     // 정지 완료: 자동연결+운전 활성
}
