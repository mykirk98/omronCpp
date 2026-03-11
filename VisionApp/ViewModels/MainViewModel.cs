using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Media.Imaging;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using VisionApp.Models;
using VisionApp.Services;

namespace VisionApp.ViewModels;

public partial class MainViewModel : ObservableObject
{
    private readonly ICameraService _cameraService;
    private readonly LogService _logService;

    [ObservableProperty]    //NOTE: ObservableProperty는 partial method로 On[PropertyName]Changed 메서드를 자동 생성해줌, public 속성과 private 필드를 동시에 생성 -> CurrentState는 public 속성, _currentState는 private 필드
    private AppState _currentState = AppState.Idle;

    [ObservableProperty]
    private bool _isLogPanelVisible = true;

    //TODO: 03/12/목 - 여기서부터 다시 작성
    public ObservableCollection<CameraTileViewModel> Cameras { get; } = [];
    public ObservableCollection<LogEntry> LogEntries { get; } = [];

    // 상태 머신에 따른 버튼 활성/비활성 제어
    public bool CanConnect => CurrentState == AppState.Idle || CurrentState == AppState.Stopped;
    public bool CanRun => CurrentState == AppState.Connected || CurrentState == AppState.Stopped;
    public bool CanStop => CurrentState == AppState.Running;

    // Constructor에서 서비스 주입 및 이벤트 구독
    public MainViewModel(ICameraService cameraService, LogService logService)
    {
        // 외부에서 ICameraService와 LogService를 주입받아 필드에 저장
        _cameraService = cameraService;
        _logService = logService;

        // 이벤트 구독 (콜백 메서드 등록)
        _logService.LogReceived += OnLogReceived;
        _cameraService.FrameReceived += OnFrameReceived;
    }

    partial void OnCurrentStateChanged(AppState value)
    {
        OnPropertyChanged(nameof(CanConnect));
        OnPropertyChanged(nameof(CanRun));
        OnPropertyChanged(nameof(CanStop));
        ConnectCommand.NotifyCanExecuteChanged();
        RunCommand.NotifyCanExecuteChanged();
        StopCommand.NotifyCanExecuteChanged();
    }

    [RelayCommand(CanExecute = nameof(CanConnect))]
    private async Task ConnectAsync()
    {
        _logService.Log("System", "카메라 자동 연결 중...");
        var cameras = await _cameraService.ConnectAsync();

        Cameras.Clear();
        foreach (var cam in cameras)
        {
            Cameras.Add(new CameraTileViewModel
            {
                Name = cam.Name,
                SerialNumber = cam.SerialNumber,
                IsConnected = cam.IsConnected
            });
        }

        CurrentState = AppState.Connected;
        _logService.Log("System", $"{cameras.Count}개 카메라 연결 완료");
    }

    [RelayCommand(CanExecute = nameof(CanRun))]
    private async Task RunAsync()
    {
        _logService.Log("System", "전체 카메라 운전 시작");
        await _cameraService.StartAllAsync();
        CurrentState = AppState.Running;
    }

    [RelayCommand(CanExecute = nameof(CanStop))]
    private async Task StopAsync()
    {
        _logService.Log("System", "전체 카메라 정지");
        await _cameraService.StopAllAsync();
        CurrentState = AppState.Stopped;
    }

    [RelayCommand]
    private void ToggleLogPanel()
    {
        IsLogPanelVisible = !IsLogPanelVisible;
    }

    private void OnFrameReceived(string cameraName, BitmapSource image, double fps)
    {
        Application.Current?.Dispatcher.Invoke(() =>
        {
            var tile = Cameras.FirstOrDefault(c => c.Name == cameraName);
            if (tile != null)
            {
                tile.CurrentImage = image;
                tile.Fps = fps;
            }
        });
    }

    private void OnLogReceived(LogEntry entry)
    {
        Application.Current?.Dispatcher.Invoke(() =>
        {
            LogEntries.Add(entry);

            while (LogEntries.Count > 500)
                LogEntries.RemoveAt(0);
        });
    }
}
