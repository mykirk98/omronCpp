using System.Windows.Media.Imaging;
using VisionApp.Models;

namespace VisionApp.Services;

public interface ICameraService
{
    /// <summary>
    /// 카메라 연결 (GigEManager.Initialize에 대응)
    /// </summary>
    Task<List<CameraInfo>> ConnectAsync();

    /// <summary>
    /// 전체 카메라 운전 시작 (GigEManager.StartAll에 대응)
    /// </summary>
    Task StartAllAsync();

    /// <summary>
    /// 전체 카메라 정지 (GigEManager.StopAll에 대응)
    /// </summary>
    Task StopAllAsync();

    /// <summary>
    /// 단일 카메라 트리거 (GigEManager.TriggerSingle에 대응)
    /// </summary>
    Task TriggerAsync(string cameraName);

    /// <summary>
    /// 프레임 수신 이벤트 (카메라이름, 이미지, 현재FPS)
    /// </summary>
    event Action<string, BitmapSource, double>? FrameReceived;
}
