using System.Windows;
using System.Windows.Controls;

namespace WpfControls;

public class SampleControl : Control
{
    static SampleControl()
    {
        DefaultStyleKeyProperty.OverrideMetadata(typeof(SampleControl), new FrameworkPropertyMetadata(typeof(SampleControl)));
    }
}
