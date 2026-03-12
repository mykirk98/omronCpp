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
        // CurrentState가 변경될 때마다 CanConnect, CanRun, CanStop의 상태도 변경되므로, 이를 알리기 위해 OnPropertyChanged 호출
        OnPropertyChanged(nameof(CanConnect));
        OnPropertyChanged(nameof(CanRun));
        OnPropertyChanged(nameof(CanStop));
        // RelayCommand의 CanExecute 상태도 변경되었음을 알리기 위해 NotifyCanExecuteChanged 호출
        ConnectCommand.NotifyCanExecuteChanged();
        RunCommand.NotifyCanExecuteChanged();
        StopCommand.NotifyCanExecuteChanged();
    }

    [RelayCommand(CanExecute = nameof(CanConnect))]
    private async Task ConnectAsync()
    {
        
        _logService.Log("System", "카메라 자동 연결 중...");
        // await -> ConnectAsync()가 완료될 때까지 기다렸다가 결과를 cameras 변수에 저장, UI 스레드가 블로킹되지 않도록 비동기적으로 실행 (안멈춤)
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
        // await -> StartAllAsync()가 완료될 때까지 기다렸다가 다음 줄 실행, UI 스레드가 블로킹되지 않도록 비동기적으로 실행 (안멈춤)
        await _cameraService.StartAllAsync();
        CurrentState = AppState.Running;
    }

    [RelayCommand(CanExecute = nameof(CanStop))]
    private async Task StopAsync()
    {
        _logService.Log("System", "전체 카메라 정지");
        // await -> StopAllAsync()가 완료될 때까지 기다렸다가 다음 줄 실행, UI 스레드가 블로킹되지 않도록 비동기적으로 실행 (안멈춤)
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
        // ? : Application.Current가 null이 아닐 때만 Dispatcher.Invoke 실행
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
