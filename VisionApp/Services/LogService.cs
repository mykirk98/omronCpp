using VisionApp.Models;

namespace VisionApp.Services;

// Observer 패턴을 사용하여 로그 메시지를 구독자에게 전달하는 LogService 클래스입니다.
public class LogService
{
    public event Action<LogEntry>? LogReceived; // ?를 사용해서 구독자가 없어도 null 오류 없이 안전하게 이벤트를 호출할 수 있도록 함.

    public void Log(string source, string message)
    {
        LogReceived?.Invoke(new LogEntry
        {
            Timestamp = DateTime.Now,
            Source = source,
            Message = message
        });
    }
}
