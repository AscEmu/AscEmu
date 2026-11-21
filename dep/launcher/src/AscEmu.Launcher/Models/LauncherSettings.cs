namespace AscEmu.Launcher.Models;

public sealed class LauncherSettings
{
    public string Portal { get; set; } = "bnet.ascemu.local:1119";
    public bool ClearCacheBeforeLaunch { get; set; } = true;
    public bool PlayPatchSound { get; set; } = true;
    public string ServerAddress { get; set; } = "127.0.0.1";
    public bool AutoConfigureHosts { get; set; } = true;
    public string CaCertificatePath { get; set; } = "";
}
