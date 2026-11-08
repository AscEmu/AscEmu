using System.Diagnostics;

namespace AscEmu.Launcher.Services;

public sealed record ClientInfo(string BinaryPath, string GameDirectory, Version Version, uint Build);

public static class ClientLocator
{
    public static string LauncherDirectory => Path.TrimEndingDirectorySeparator(AppContext.BaseDirectory);
    public static readonly string[] BinaryNames = ["Wow.exe", "WowB.exe"];
    public static string ExpectedBinaryNames => string.Join(" or ", BinaryNames);

    public static ClientInfo? Locate()
    {
        var binary = BinaryNames.Select(name => Path.Combine(LauncherDirectory, name)).FirstOrDefault(File.Exists);
        if (binary == null)
            return null;

        var versionInfo = FileVersionInfo.GetVersionInfo(binary);
        var parsed = ParseVersion(versionInfo);
        return new(binary, LauncherDirectory, parsed.version, parsed.build);
    }

    static (Version version, uint build) ParseVersion(FileVersionInfo info)
    {
        if (Version.TryParse(info.FileVersion, out var fileVersion) && fileVersion != null)
        {
            var build = fileVersion.Revision > 0 ? (uint)fileVersion.Revision : (uint)Math.Max(0, info.FilePrivatePart);
            if (build > 60000)
                return (fileVersion, build);
        }

        var buildPart = info.FileBuildPart;
        var privatePart = info.FilePrivatePart;
        if (buildPart >= 6553 && privatePart < ushort.MaxValue)
            return (new Version(info.FileMajorPart / 100, info.FileMajorPart % 100, info.FileMinorPart, buildPart * 10 + privatePart), (uint)(buildPart * 10 + privatePart));

        return (new Version(info.FileMajorPart, info.FileMinorPart, info.FileBuildPart, info.FilePrivatePart), (uint)Math.Max(0, privatePart));
    }
}
