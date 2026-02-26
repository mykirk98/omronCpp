namespace VisionApp.Models;

public class CameraInfo
{
    public string Name { get; set; } = string.Empty;
    public string SerialNumber { get; set; } = string.Empty;
    public bool IsConnected { get; set; }
    public double Fps { get; set; }
}
