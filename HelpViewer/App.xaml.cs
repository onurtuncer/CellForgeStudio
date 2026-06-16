using System;
using System.Windows;

namespace HelpViewer;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        nint parentHwnd = 0;
        for (int i = 0; i < e.Args.Length - 1; i++)
        {
            if (e.Args[i].Equals("--parent-hwnd", StringComparison.OrdinalIgnoreCase))
            {
                string val = e.Args[i + 1];
                parentHwnd = (nint)Convert.ToInt64(
                    val.StartsWith("0x", StringComparison.OrdinalIgnoreCase) ? val[2..] : val,
                    val.StartsWith("0x", StringComparison.OrdinalIgnoreCase) ? 16 : 10);
                break;
            }
        }

        new MainWindow(parentHwnd).Show();
    }
}
