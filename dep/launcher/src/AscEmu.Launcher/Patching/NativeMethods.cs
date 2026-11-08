using System.Runtime.InteropServices;

namespace AscEmu.Launcher.Patching;

internal static class NativeMethods
{
    internal const uint CreateSuspended = 0x00000004;
    internal const uint ProcessVmOperation = 0x0008;
    internal const uint ProcessVmRead = 0x0010;
    internal const uint ProcessVmWrite = 0x0020;
    internal const uint ProcessQueryInformation = 0x0400;
    internal const uint PageExecuteReadWrite = 0x40;
    internal const uint MemCommit = 0x1000;

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)] internal struct StartupInfo { public int cb; public string? lpReserved; public string? lpDesktop; public string? lpTitle; public int dwX; public int dwY; public int dwXSize; public int dwYSize; public int dwXCountChars; public int dwYCountChars; public int dwFillAttribute; public int dwFlags; public short wShowWindow; public short cbReserved2; public nint lpReserved2; public nint hStdInput; public nint hStdOutput; public nint hStdError; }
    [StructLayout(LayoutKind.Sequential)] internal struct ProcessInformation { public nint hProcess; public nint hThread; public uint dwProcessId; public uint dwThreadId; }
    [StructLayout(LayoutKind.Sequential)] internal struct MemoryBasicInformation { public nint BaseAddress; public nint AllocationBase; public uint AllocationProtect; public ushort PartitionId; public nint RegionSize; public uint State; public uint Protect; public uint Type; }

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)] internal static extern bool CreateProcessW(string? applicationName, string commandLine, nint processAttributes, nint threadAttributes, bool inheritHandles, uint creationFlags, nint environment, string? currentDirectory, ref StartupInfo startupInfo, out ProcessInformation processInformation);
    [DllImport("kernel32.dll", SetLastError = true)] internal static extern bool ReadProcessMemory(nint process, nint address, byte[] buffer, int size, out nint read);
    [DllImport("kernel32.dll", SetLastError = true)] internal static extern bool WriteProcessMemory(nint process, nint address, byte[] buffer, int size, out nint written);
    [DllImport("kernel32.dll", SetLastError = true)] internal static extern bool VirtualProtectEx(nint process, nint address, nint size, uint newProtect, out uint oldProtect);
    [DllImport("kernel32.dll", SetLastError = true)] internal static extern nint VirtualQueryEx(nint process, nint address, out MemoryBasicInformation info, nint length);
    [DllImport("kernel32.dll")] internal static extern uint ResumeThread(nint thread);
    [DllImport("kernel32.dll")] internal static extern bool CloseHandle(nint handle);
}
