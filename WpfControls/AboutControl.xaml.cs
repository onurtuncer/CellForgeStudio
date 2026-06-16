using System;
using System.Windows;
using System.Windows.Controls;

namespace WpfControls;

public partial class AboutControl : UserControl
{
    public event Action? CloseRequested;

    public AboutControl()
    {
        InitializeComponent();
    }

    private void Close_Click(object sender, RoutedEventArgs e)
    {
        CloseRequested?.Invoke();
    }
}
