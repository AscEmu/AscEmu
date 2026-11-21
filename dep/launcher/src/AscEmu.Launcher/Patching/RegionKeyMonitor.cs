using System.Diagnostics;
using System.Media;
using System.Runtime.InteropServices;
using AscEmu.Launcher.Services;

namespace AscEmu.Launcher.Patching;

// Writes the AscEmu public key into every region-key table of the client and keeps scanning: the client holds a
// table when it reaches the logon and may build or renew one before it connects to the world server.
public sealed class RegionKeyMonitor : IDisposable
{
    const int RecordSize = 0x28;
    const int RecordWords = RecordSize / sizeof(uint);
    const int KeyOffset = 0x04;
    const int KeySize = 32;
    const int FlagOffset = 0x24;
    const int MinRecordCount = 8;
    const int MaxRecordCount = 32;
    const long MaximumRegionSize = 64L * 1024L * 1024L;

    readonly nint _processHandle;
    readonly int _processId;
    readonly bool _playSound;
    readonly CancellationTokenSource _cts = new();
    readonly Dictionary<nint, int> _tables = new();
    byte[] _buffer = new byte[1024 * 1024];
    bool _playedPatchSound;
    bool _loggedScanTime;

    public event Action<string>? StatusChanged;
    public event Action? Patched;

    public RegionKeyMonitor(nint processHandle, int processId, bool playSound)
    {
        _processHandle = processHandle;
        _processId = processId;
        _playSound = playSound;
    }

    public void Start() => _ = Task.Run(() => RunAsync(_cts.Token));

    public void Dispose()
    {
        _cts.Cancel();
        _cts.Dispose();
    }

    async Task RunAsync(CancellationToken token)
    {
        var memory = new ProcessMemory(_processHandle);
        Report("Attached to the client; waiting for region-key tables...");

        while (!token.IsCancellationRequested)
        {
            try
            {
                if (!IsProcessAlive())
                {
                    LauncherLog.Write("Client process ended");
                    return;
                }

                var scanStart = Stopwatch.GetTimestamp();
                var writtenKeys = 0;
                foreach (var (tableBase, count) in FindTables(memory))
                {
                    if (!_tables.TryGetValue(tableBase, out var known) || known != count)
                    {
                        _tables[tableBase] = count;
                        LauncherLog.Write($"Region-key table with ids 1..{count} at 0x{tableBase.ToInt64():X}");
                    }

                    var written = PatchTable(memory, tableBase, count);
                    if (written > 0)
                        LauncherLog.Write($"Wrote {written} of {count} keys at 0x{tableBase.ToInt64():X}");
                    writtenKeys += written;
                }

                var scanMs = Stopwatch.GetElapsedTime(scanStart).TotalMilliseconds;
                if (writtenKeys > 0 || !_loggedScanTime)
                {
                    LauncherLog.Write($"Scan took {scanMs:F0} ms");
                    _loggedScanTime = true;
                }

                if (writtenKeys > 0)
                {
                    Report($"Patched {writtenKeys} region public keys ({_tables.Count} table(s) found)");
                    if (_playSound && !_playedPatchSound)
                    {
                        try { SystemSounds.Asterisk.Play(); } catch { }
                        _playedPatchSound = true;
                    }
                    Patched?.Invoke();
                }

                await Task.Delay(100, token);
            }
            catch (OperationCanceledException)
            {
                return;
            }
            catch
            {
                await Task.Delay(500, token);
            }
        }
    }

    void Report(string text)
    {
        LauncherLog.Write(text);
        StatusChanged?.Invoke(text);
    }

    List<(nint Base, int Count)> FindTables(ProcessMemory memory)
    {
        var tables = new List<(nint Base, int Count)>();
        foreach (var region in memory.ReadableRegions())
        {
            if (!IsWritable(region.Protect) || region.Size < MinRecordCount * RecordSize || region.Size > MaximumRegionSize)
                continue;

            var size = (int)region.Size;
            if (_buffer.Length < size)
                _buffer = new byte[size];
            if (!memory.Read(region.Base, _buffer, size))
                continue;

            var bytes = _buffer.AsSpan(0, size);
            var words = MemoryMarshal.Cast<byte, uint>(bytes);
            var lastStart = words.Length - MinRecordCount * RecordWords;
            for (var index = 0; index <= lastStart; ++index)
            {
                // ids 1 and 2 one record apart, the full check only then
                if (words[index] != 1 || words[index + RecordWords] != 2)
                    continue;

                var count = CountRecords(bytes[(index * sizeof(uint))..]);
                if (count < MinRecordCount)
                    continue;

                tables.Add((region.Base + index * sizeof(uint), count));
                index += count * RecordWords - 1;
            }
        }
        return tables;
    }

    // records with the ids 1..n in a row; the table ends where the next id does not follow
    static int CountRecords(ReadOnlySpan<byte> bytes)
    {
        var count = 0;
        while (count < MaxRecordCount && (count + 1) * RecordSize <= bytes.Length)
        {
            var record = bytes.Slice(count * RecordSize, RecordSize);
            if (BitConverter.ToUInt32(record[..4]) != (uint)(count + 1) || record[FlagOffset] > 1)
                break;
            ++count;
        }
        return count;
    }

    // number of keys that were replaced
    static int PatchTable(ProcessMemory memory, nint tableBase, int count)
    {
        var written = 0;
        var current = new byte[KeySize];
        for (var index = 0; index < count; ++index)
        {
            var keyAddress = tableBase + index * RecordSize + KeyOffset;
            if (!memory.Read(keyAddress, current) || current.AsSpan().SequenceEqual(ForeverPatchProfile.AscEmuEd25519PublicKey))
                continue;

            if (memory.Write(keyAddress, ForeverPatchProfile.AscEmuEd25519PublicKey))
                ++written;
        }
        return written;
    }

    static bool IsWritable(uint protection)
    {
        if ((protection & 0x100u) != 0)
            return false;
        return (protection & 0xFFu) is 0x04u or 0x08u or 0x40u or 0x80u;
    }

    bool IsProcessAlive()
    {
        try { return !Process.GetProcessById(_processId).HasExited; }
        catch { return false; }
    }
}
