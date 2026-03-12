using System.Windows.Media.Imaging;
using CommunityToolkit.Mvvm.ComponentModel;

namespace VisionApp.ViewModels;

public partial class CameraTileViewModel : ObservableObject
{
    [ObservableProperty]
    private string _name = string.Empty;

    [ObservableProperty]
    private string _serialNumber = string.Empty;

    [ObservableProperty]
    private bool _isConnected;

    [ObservableProperty]
    private double _fps;

    [ObservableProperty]
    private BitmapSource? _currentImage;

    public string FpsDisplay => $"FPS: {Fps:F1}";

    partial void OnFpsChanged(double value)
    {
        OnPropertyChanged(nameof(FpsDisplay));
    }
}
