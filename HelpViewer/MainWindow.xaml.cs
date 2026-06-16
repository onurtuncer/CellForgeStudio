using System;
using System.Windows;
using System.Windows.Interop;

namespace HelpViewer;

public partial class MainWindow : Window
{
    private readonly nint _parentHwnd;

    public MainWindow(nint parentHwnd)
    {
        _parentHwnd = parentHwnd;
        InitializeComponent();
        SourceInitialized += OnSourceInitialized;
        helpControl.CloseRequested += Close;
    }

    private void OnSourceInitialized(object? sender, EventArgs e)
    {
        if (_parentHwnd == 0) return;
        // Own by the native parent: stays on top of it, minimises together,
        // shares its taskbar button.
        new WindowInteropHelper(this).Owner = _parentHwnd;
    }
}
