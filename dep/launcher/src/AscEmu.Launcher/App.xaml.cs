using System.Windows;

namespace AscEmu.Launcher;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        // AscEmu Forever currently uses the known-good "none" launch path by default.
        // --forever-test none remains accepted for compatibility with the old development launcher.
        for (var i = 0; i < e.Args.Length; ++i)
        {
            if (!string.Equals(e.Args[i], "--forever-test", StringComparison.OrdinalIgnoreCase))
                continue;

            if (i + 1 >= e.Args.Length || !string.Equals(e.Args[i + 1], "none", StringComparison.OrdinalIgnoreCase))
            {
                MessageBox.Show("Only '--forever-test none' is currently supported.", "AscEmu Launcher", MessageBoxButton.OK, MessageBoxImage.Error);
                Shutdown(2);
                return;
            }

            ++i;
        }

        MainWindow = new MainWindow();
        MainWindow.Show();
    }
}
