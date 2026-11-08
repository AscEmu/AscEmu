namespace AscEmu.Launcher.Models;

public sealed class LauncherSettings
{
    public string Portal { get; set; } = "127.0.0.1:1119";
    public bool ClearCacheBeforeLaunch { get; set; } = true;
    public bool PlayPatchSound { get; set; } = true;
}
