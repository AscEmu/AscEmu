namespace AscEmu.Launcher.Core;

public static class BuildProfiles
{
    public const uint MinSupportedBuild = 69893;
    public const uint MaxSupportedBuild = 70862;

    public static bool IsSupported(uint build) => build >= MinSupportedBuild && build <= MaxSupportedBuild;
    public static string GetLabel(uint build) => IsSupported(build) ? $"Forever 1.60.1 · {build}" : $"Unsupported build {build}";
}
