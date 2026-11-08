namespace AscEmu.Launcher.Patching;

internal static class ForeverClientPatcher
{
    public static void PatchPreLaunch(nint processHandle, uint build, Action<string> status)
    {
        if (build != ForeverPatchProfile.VerifiedBuild) throw new NotSupportedException($"Direct Forever memory patching is only verified for build {ForeverPatchProfile.VerifiedBuild}; found {build}.");
        var memory = new ProcessMemory(processHandle);
        PatchExact(memory, ForeverPatchProfile.PortalSuffix, ForeverPatchProfile.PortalSuffixReplacement, "login portal suffix", status);
        PatchExact(memory, ForeverPatchProfile.LauncherRegistry, Pad(ForeverPatchProfile.LauncherRegistryReplacement, ForeverPatchProfile.LauncherRegistry.Length), "launcher registry path", status);
        var keyAddress = memory.Find(ForeverPatchProfile.EmbeddedEd25519Prefix);
        if (keyAddress == 0) throw new InvalidOperationException("Forever GameCrypto Ed25519 key was not found in the client process.");
        if (!memory.Write(keyAddress, ForeverPatchProfile.AscEmuEd25519PublicKey)) throw new InvalidOperationException("Failed to patch Forever GameCrypto Ed25519 public key.");
        status("Client crypto and portal patches applied");
    }

    static void PatchExact(ProcessMemory memory, byte[] search, byte[] replacement, string name, Action<string> status)
    {
        var address = memory.Find(search);
        if (address == 0) throw new InvalidOperationException($"Could not locate {name} in the client process.");
        if (!memory.Write(address, replacement)) throw new InvalidOperationException($"Could not patch {name}.");
        status($"Patched {name}");
    }

    static byte[] Pad(byte[] source, int size)
    {
        if (source.Length > size) throw new InvalidOperationException("Replacement registry path is longer than the embedded client path.");
        var result = new byte[size]; Array.Copy(source, result, source.Length); return result;
    }
}
