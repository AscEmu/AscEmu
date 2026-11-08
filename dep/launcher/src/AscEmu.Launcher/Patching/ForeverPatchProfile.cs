using System.Text;

namespace AscEmu.Launcher.Patching;

internal static class ForeverPatchProfile
{
    public const uint VerifiedBuild = 69893;
    public static readonly byte[] EmbeddedEd25519Prefix = [0x15,0xD6,0x18,0xBD,0x7D,0xB5,0x77,0xBD,0x9A,0x8D,0x45,0x76,0x9C,0x59,0xE4,0xFC];
    public static readonly byte[] AscEmuEd25519PublicKey = [0x02,0x59,0x6F,0x0D,0x0C,0x06,0x1A,0x8B,0x30,0x74,0x59,0x88,0xFD,0x72,0xC5,0x9E,0x29,0xEC,0x36,0x7F,0xB0,0xF3,0x41,0xF2,0x8E,0x0F,0x08,0xD0,0x37,0xBA,0xFC,0x69];
    public static readonly byte[] PortalSuffix = Encoding.UTF8.GetBytes(".actual.battle.net\0");
    public static readonly byte[] PortalSuffixReplacement = new byte[PortalSuffix.Length];
    public static readonly byte[] LauncherRegistry = Encoding.UTF8.GetBytes("Software\\Blizzard Entertainment\\Battle.net\\Launch Options\\");
    public static readonly byte[] LauncherRegistryReplacement = Encoding.UTF8.GetBytes("Software\\AscEmu\\Battle.net\\Launch Options\\");
}
