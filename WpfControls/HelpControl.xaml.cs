using System;
using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;

namespace WpfControls;

public partial class HelpControl : UserControl
{
    public event Action? CloseRequested;

    public HelpControl()
    {
        InitializeComponent();
        Loaded += OnLoaded;
    }

    private async void OnLoaded(object sender, RoutedEventArgs e)
    {
        try
        {
            await webView.EnsureCoreWebView2Async(null);

            string helpDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) ?? "";
            string indexPath = Path.Combine(helpDir, "help", "index.html");

            if (File.Exists(indexPath))
                webView.CoreWebView2.Navigate(new Uri(indexPath).AbsoluteUri);
            else
                webView.CoreWebView2.NavigateToString(FallbackHtml);
        }
        catch (Exception ex)
        {
            webView.Visibility = Visibility.Collapsed;
            errorText.Text = $"Could not load WebView2: {ex.Message}\n\nEnsure the WebView2 Runtime is installed.";
            errorText.Visibility = Visibility.Visible;
        }
    }

    private void Close_Click(object sender, RoutedEventArgs e) => CloseRequested?.Invoke();

    private const string FallbackHtml =
        "<html><body style='font-family:sans-serif;padding:24px'>" +
        "<h2>Help</h2><p>Help files not found next to the executable.</p>" +
        "</body></html>";
}
