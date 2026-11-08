using AscEmu.Launcher.Core;
using AscEmu.Launcher.Models;
using AscEmu.Launcher.Patching;

namespace AscEmu.Launcher.Services;

public sealed class RunningGame : IDisposable
{
    public RegionKeyMonitor? RegionKeys { get; init; }
    public void Dispose() => RegionKeys?.Dispose();
}

public static class GameLauncher
{
    public static RunningGame Launch(ClientInfo client, LauncherSettings settings, Action<string> status)
    {
        if (!BuildProfiles.IsSupported(client.Build)) throw new NotSupportedException($"Supported Forever builds are {BuildProfiles.MinSupportedBuild} through {BuildProfiles.MaxSupportedBuild}. Detected {client.Build}.");
        PortalService.WritePortal(client.GameDirectory, settings.Portal);
        if (settings.ClearCacheBeforeLaunch)
        {
            var cache = Path.Combine(client.GameDirectory, "Cache");
            try { if (Directory.Exists(cache)) Directory.Delete(cache, true); } catch { }
        }
        var startup = new NativeMethods.StartupInfo { cb = System.Runtime.InteropServices.Marshal.SizeOf<NativeMethods.StartupInfo>() };
        var commandLine = $"\"{client.BinaryPath}\" -config Config.wtf";
        if (!NativeMethods.CreateProcessW(client.BinaryPath, commandLine, 0, 0, false, NativeMethods.CreateSuspended, 0, client.GameDirectory, ref startup, out var process))
            throw new System.ComponentModel.Win32Exception(System.Runtime.InteropServices.Marshal.GetLastWin32Error(), $"Could not launch {Path.GetFileName(client.BinaryPath)}.");
        try
        {
            status("Forever launch mode: none (no pre-launch memory patches)");
            var monitor = new RegionKeyMonitor(process.hProcess, (int)process.dwProcessId, settings.PlayPatchSound);
            monitor.StatusChanged += status;
            NativeMethods.ResumeThread(process.hThread);
            monitor.Start();
            NativeMethods.CloseHandle(process.hThread);
            return new RunningGame { RegionKeys = monitor };
        }
        catch
        {
            NativeMethods.CloseHandle(process.hThread); NativeMethods.CloseHandle(process.hProcess); throw;
        }
    }
}
