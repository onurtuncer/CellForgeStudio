using System;
using System.Windows;
using System.Windows.Interop;

namespace AboutViewer;

public partial class MainWindow : Window
{
    private readonly nint _parentHwnd;

    public string VersionText   { get; }
    public string CopyrightText { get; }

    public MainWindow(nint parentHwnd, string version, string copyright)
    {
        _parentHwnd   = parentHwnd;
        VersionText   = string.IsNullOrEmpty(version)   ? "" : $"Version {version}";
        CopyrightText = copyright;
        DataContext   = this;
        InitializeComponent();
        SourceInitialized += OnSourceInitialized;
    }

    private void OnSourceInitialized(object? sender, EventArgs e)
    {
        if (_parentHwnd == 0) return;
        // Own by the native parent: keeps this window on top, shares its
        // taskbar button, and gives CenterOwner a target to centre against.
        new WindowInteropHelper(this).Owner = _parentHwnd;
    }

    private void Close_Click(object sender, RoutedEventArgs e) => Close();
}
