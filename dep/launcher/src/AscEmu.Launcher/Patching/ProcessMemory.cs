namespace AscEmu.Launcher.Patching;

internal sealed class ProcessMemory(nint processHandle)
{
    public bool Read(nint address, byte[] buffer) => NativeMethods.ReadProcessMemory(processHandle, address, buffer, buffer.Length, out var read) && read == buffer.Length;

    public bool Write(nint address, ReadOnlySpan<byte> data)
    {
        var bytes = data.ToArray();
        if (!NativeMethods.VirtualProtectEx(processHandle, address, bytes.Length, NativeMethods.PageExecuteReadWrite, out var oldProtect)) return false;
        var ok = NativeMethods.WriteProcessMemory(processHandle, address, bytes, bytes.Length, out var written) && written == bytes.Length;
        NativeMethods.VirtualProtectEx(processHandle, address, bytes.Length, oldProtect, out _);
        return ok;
    }

    public nint Find(ReadOnlySpan<byte> pattern, long maxAddress = 0x00007FFFFFFEFFFF)
    {
        long address = 0x10000;
        const int chunkSize = 2 * 1024 * 1024;
        while (address < maxAddress)
        {
            if (NativeMethods.VirtualQueryEx(processHandle, (nint)address, out var mbi, System.Runtime.InteropServices.Marshal.SizeOf<NativeMethods.MemoryBasicInformation>()) == 0) break;
            var regionBase = mbi.BaseAddress.ToInt64();
            var regionSize = mbi.RegionSize.ToInt64();
            if (regionSize <= 0) break;
            if (mbi.State == NativeMethods.MemCommit && IsReadable(mbi.Protect))
            {
                for (long offset = 0; offset < regionSize; offset += Math.Max(1, chunkSize - pattern.Length))
                {
                    var count = (int)Math.Min(chunkSize, regionSize - offset);
                    if (count < pattern.Length) break;
                    var buffer = new byte[count];
                    if (!NativeMethods.ReadProcessMemory(processHandle, (nint)(regionBase + offset), buffer, count, out var read) || read.ToInt64() < pattern.Length) continue;
                    var limit = (int)read - pattern.Length;
                    for (var i = 0; i <= limit; ++i)
                    {
                        if (buffer.AsSpan(i, pattern.Length).SequenceEqual(pattern)) return (nint)(regionBase + offset + i);
                    }
                }
            }
            address = regionBase + regionSize;
        }
        return 0;
    }

    public IEnumerable<(nint Base, long Size, uint Protect)> ReadableRegions()
    {
        long address = 0x10000;
        const long maxAddress = 0x00007FFFFFFEFFFF;
        while (address < maxAddress)
        {
            if (NativeMethods.VirtualQueryEx(processHandle, (nint)address, out var mbi, System.Runtime.InteropServices.Marshal.SizeOf<NativeMethods.MemoryBasicInformation>()) == 0) yield break;
            var regionBase = mbi.BaseAddress.ToInt64(); var regionSize = mbi.RegionSize.ToInt64();
            if (regionSize <= 0) yield break;
            if (mbi.State == NativeMethods.MemCommit && IsReadable(mbi.Protect)) yield return ((nint)regionBase, regionSize, mbi.Protect);
            address = regionBase + regionSize;
        }
    }

    static bool IsReadable(uint protection)
    {
        var p = protection & 0xFFu;
        return p is 0x02 or 0x04 or 0x08 or 0x20 or 0x40 or 0x80;
    }
}
