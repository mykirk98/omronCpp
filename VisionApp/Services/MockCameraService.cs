using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using VisionApp.Models;

namespace VisionApp.Services;

public class MockCameraService : ICameraService
{
    private readonly List<CameraInfo> _cameras =
    [
        new CameraInfo { Name = "TOP",    SerialNumber = "25E9151" },
        new CameraInfo { Name = "BOTTOM", SerialNumber = "23F8382" },
        new CameraInfo { Name = "SIDE",   SerialNumber = "MOCK001" },
    ];

    private Timer? _timer;
    private readonly Random _random = new();

    public event Action<string, BitmapSource, double>? FrameReceived;

    public Task<List<CameraInfo>> ConnectAsync()
    {
        foreach (var cam in _cameras)
            cam.IsConnected = true;

        return Task.FromResult(_cameras);
    }

    public Task StartAllAsync()
    {
        _timer = new Timer(OnTimerTick, null, 0, 100);
        return Task.CompletedTask;
    }

    public Task StopAllAsync()
    {
        _timer?.Dispose();
        _timer = null;
        return Task.CompletedTask;
    }

    public Task TriggerAsync(string cameraName)
    {
        return Task.CompletedTask;
    }

    private void OnTimerTick(object? state)
    {
        foreach (var cam in _cameras)
        {
            double fps = 9.0 + _random.NextDouble() * 2.0;

            Application.Current?.Dispatcher.Invoke(() =>
            {
                var bmp = GenerateMockImage(640, 480);
                bmp.Freeze();
                FrameReceived?.Invoke(cam.Name, bmp, fps);
            });
        }
    }

    private WriteableBitmap GenerateMockImage(int width, int height)
    {
        var bmp = new WriteableBitmap(width, height, 96, 96, PixelFormats.Bgr24, null);
        byte[] pixels = new byte[width * height * 3];
        _random.NextBytes(pixels);
        bmp.WritePixels(new Int32Rect(0, 0, width, height), pixels, width * 3, 0);
        return bmp;
    }
}
