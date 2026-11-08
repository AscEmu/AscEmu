using System.Text.Json;
using AscEmu.Launcher.Models;

namespace AscEmu.Launcher.Services;

public static class SettingsService
{
    static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true, PropertyNameCaseInsensitive = true };
    public static string SettingsPath => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "AscEmu", "Launcher", "launcher.json");
    // a launcher.json next to the launcher wins over the user profile
    public static string LocalSettingsPath => Path.Combine(ClientLocator.LauncherDirectory, "launcher.json");
    public static string LoadedPath { get; private set; } = string.Empty;

    public static LauncherSettings Load()
    {
        foreach (var path in new[] { LocalSettingsPath, SettingsPath })
        {
            try
            {
                if (!File.Exists(path))
                    continue;
                var settings = JsonSerializer.Deserialize<LauncherSettings>(File.ReadAllText(path), JsonOptions);
                if (settings == null)
                    continue;
                LoadedPath = path;
                return settings;
            }
            catch { }
        }
        return new();
    }

    public static void Save(LauncherSettings settings)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(SettingsPath)!);
        File.WriteAllText(SettingsPath, JsonSerializer.Serialize(settings, JsonOptions));
    }
}
