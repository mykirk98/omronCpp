using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace VisionApp.Converters;

public class BoolToColorConverter : IValueConverter
{
    // 데이터 -> UI (바인딩할 때 호출)
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture) //NOTE: Type: 런타임에 어떤 타입인지 확인할 수 있는 정보, object: 어떤 타입이든 받을 수 있는 매개변수
    {
        if (value is true)
            return Brushes.LimeGreen;
        else
            return Brushes.Red;
    }

    // UI -> 데이터 (양방향 바인딩에서 호출)
    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        => throw new NotSupportedException();   // ConvertBack은 양방향 바인딩에서 사용되며, 이 경우에는 필요하지 않으므로 예외를 던짐
}
