using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace VisionApp.Converters;

// bool 값을 WPF의 Visibility enum으로 변환해 XAML에서 UI 요소의 표시 여부를 바인딩으로 제어할 수 있도록 하는 컨버터
// 로그 패널 숨길 때 사용
public class BoolToVisibilityConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is true)
            return Visibility.Visible;  // 보임
        else
            return Visibility.Collapsed;    // 숨김 (공간도 차지하지 않음)
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if(value is Visibility.Visible)
            return true;
        else
            return false;
    }
}
