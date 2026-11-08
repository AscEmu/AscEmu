using System.Text.RegularExpressions;

namespace AscEmu.Launcher.Services;

public static partial class PortalService
{
    [GeneratedRegex("(?im)^\\s*SET\\s+portal\\s+\"[^\"]*\"\\s*$")]
    private static partial Regex PortalLineRegex();

    public static void WritePortal(string gameDirectory, string portal)
    {
        if (string.IsNullOrWhiteSpace(portal)) throw new ArgumentException("Portal cannot be empty.");
        var wtf = Path.Combine(gameDirectory, "WTF");
        Directory.CreateDirectory(wtf);
        var config = Path.Combine(wtf, "Config.wtf");
        var text = File.Exists(config) ? File.ReadAllText(config) : string.Empty;
        var line = $"SET portal \"{portal.Trim()}\"";
        text = PortalLineRegex().IsMatch(text) ? PortalLineRegex().Replace(text, line, 1) : (text.TrimEnd() + Environment.NewLine + line + Environment.NewLine);
        File.WriteAllText(config, text);
    }
}
