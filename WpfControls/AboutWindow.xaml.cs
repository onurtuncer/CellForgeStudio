using System.Windows;

namespace WpfControls;

public partial class AboutWindow : Window
{
    public AboutWindow()
    {
        InitializeComponent();
        aboutControl.CloseRequested += () => this.Close();
    }
}
