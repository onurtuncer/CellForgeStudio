using System;
using System.Windows;

namespace AboutViewer;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        nint  parentHwnd = 0;
        string version   = "";
        string copyright = "";

        for (int i = 0; i < e.Args.Length - 1; i++)
        {
            switch (e.Args[i].ToLowerInvariant())
            {
                case "--parent-hwnd":
                    var val = e.Args[++i];
                    parentHwnd = (nint)Convert.ToInt64(
                        val.StartsWith("0x", StringComparison.OrdinalIgnoreCase) ? val[2..] : val,
                        val.StartsWith("0x", StringComparison.OrdinalIgnoreCase) ? 16 : 10);
                    break;
                case "--version":   version   = e.Args[++i]; break;
                case "--copyright": copyright = e.Args[++i]; break;
            }
        }

        new MainWindow(parentHwnd, version, copyright).Show();
    }
}
