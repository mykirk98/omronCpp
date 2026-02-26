using System.Windows;
using VisionApp.Services;
using VisionApp.ViewModels;

namespace VisionApp;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();

        var logService = new LogService();  // 로그 서비스 인스턴스 생성
        var cameraService = new MockCameraService();    // 카메라 서비스 인스턴스 생성 (실제 구현에서는 실제 서비스로 교체)
        DataContext = new MainViewModel(cameraService, logService); // 뷰모델에 서비스 주입
    }
}
