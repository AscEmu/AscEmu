using System;
using System.IO;
using System.Text;

namespace AscEmu.Launcher;

public static class LauncherLog
{
    private static readonly object Sync = new();
    private static readonly string LogDirectory = Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
        "AscEmu",
        "Launcher");
    private static readonly string LogFilePath = Path.Combine(LogDirectory, "launcher.log");

    public static void Write(string message)
    {
        try
        {
            lock (Sync)
            {
                Directory.CreateDirectory(LogDirectory);
                var line = $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] {message}{Environment.NewLine}";
                File.AppendAllText(LogFilePath, line, Encoding.UTF8);
            }
        }
        catch
        {
            // Logging should not throw exceptions, so we silently ignore any errors here.
        }
    }
}
