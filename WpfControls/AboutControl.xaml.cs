using System;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;

namespace WpfControls;

public partial class AboutControl : UserControl
{
    public event Action? CloseRequested;

    public string VersionText { get; } =
        $"Version {Assembly.GetExecutingAssembly().GetName().Version?.ToString(3) ?? "unknown"}";

    public string CopyrightText { get; } =
        ((AssemblyCopyrightAttribute?)Attribute.GetCustomAttribute(
            Assembly.GetExecutingAssembly(),
            typeof(AssemblyCopyrightAttribute)))?.Copyright ?? string.Empty;

    public AboutControl()
    {
        DataContext = this;
        InitializeComponent();
    }

    private void Close_Click(object sender, RoutedEventArgs e)
    {
        CloseRequested?.Invoke();
    }
}
