using System.Diagnostics;
using System.Media;

namespace AscEmu.Launcher.Patching;

public sealed class RegionKeyMonitor : IDisposable
{
    const int RecordSize = 0x28;
    const int KeyOffset = 0x04;
    const int KeySize = 32;
    const int FlagOffset = 0x24;
    const int MinRecordCount = 8;
    const int MaxRecordCount = 32;
    const long MaximumRegionSize = 64L * 1024L * 1024L;
    int _recordCount;

    readonly nint _processHandle;
    readonly int _processId;
    readonly bool _playSound;
    readonly CancellationTokenSource _cts = new();
    bool _playedPatchSound;

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
        nint tableBase = 0;
        var announcedWaiting = false;

        StatusChanged?.Invoke("Attached to the client; waiting for region-key table...");

        while (!token.IsCancellationRequested)
        {
            try
            {
                if (!IsProcessAlive())
                    return;

                if (tableBase != 0 && !ValidateTableAt(memory, tableBase))
                {
                    StatusChanged?.Invoke("Region-key table moved/rebuilt; rescanning...");
                    tableBase = 0;
                    announcedWaiting = false;
                }

                if (tableBase == 0)
                {
                    var matches = FindTables(memory);
                    if (matches.Count == 1)
                    {
                        tableBase = matches[0].Base;
                        _recordCount = matches[0].Count;
                        StatusChanged?.Invoke($"Found RegionGroup 1..{_recordCount} table at 0x{tableBase.ToInt64():X}");
                        announcedWaiting = false;
                    }
                    else if (matches.Count > 1)
                    {
                        StatusChanged?.Invoke($"Refusing to patch: found {matches.Count} exact region-key tables");
                    }
                    else if (!announcedWaiting)
                    {
                        StatusChanged?.Invoke("Region keys are not built yet; listener is waiting...");
                        announcedWaiting = true;
                    }
                }

                if (tableBase != 0 && TableNeedsPatch(memory, tableBase))
                {
                    if (PatchTable(memory, tableBase))
                    {
                        StatusChanged?.Invoke($"Patched all {_recordCount} Forever region public keys");
                        if (_playSound && !_playedPatchSound)
                        {
                            try { SystemSounds.Asterisk.Play(); } catch { }
                            _playedPatchSound = true;
                        }
                        Patched?.Invoke();
                    }
                    else
                    {
                        tableBase = 0;
                    }
                }

                await Task.Delay(250, token);
            }
            catch (OperationCanceledException)
            {
                return;
            }
            catch
            {
                tableBase = 0;
                await Task.Delay(500, token);
            }
        }
    }

    List<(nint Base, int Count)> FindTables(ProcessMemory memory)
    {
        var matches = new List<(nint Base, int Count)>();
        foreach (var region in memory.ReadableRegions())
        {
            if (!IsWritable(region.Protect) || region.Size < MinRecordCount * RecordSize || region.Size > MaximumRegionSize)
                continue;

            if (region.Size > int.MaxValue)
                continue;

            var bytes = new byte[(int)region.Size];
            if (!memory.Read(region.Base, bytes))
                continue;

            for (var offset = 0; offset + MinRecordCount * RecordSize <= bytes.Length; offset += 4)
            {
                var count = CountRecords(bytes.AsSpan(offset));
                if (count < MinRecordCount)
                    continue;

                matches.Add((region.Base + offset, count));
                offset += count * RecordSize - 4;
            }
        }
        return matches;
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

    bool ValidateTableAt(ProcessMemory memory, nint tableBase)
    {
        var bytes = new byte[(_recordCount + 1) * RecordSize];
        return memory.Read(tableBase, bytes) && CountRecords(bytes) == _recordCount;
    }

    bool TableNeedsPatch(ProcessMemory memory, nint tableBase)
    {
        for (var index = 0; index < _recordCount; ++index)
        {
            var current = new byte[KeySize];
            var keyAddress = tableBase + index * RecordSize + KeyOffset;
            if (!memory.Read(keyAddress, current) || !current.SequenceEqual(ForeverPatchProfile.AscEmuEd25519PublicKey))
                return true;
        }
        return false;
    }

    bool PatchTable(ProcessMemory memory, nint tableBase)
    {
        for (var index = 0; index < _recordCount; ++index)
        {
            var keyAddress = tableBase + index * RecordSize + KeyOffset;
            if (!memory.Write(keyAddress, ForeverPatchProfile.AscEmuEd25519PublicKey))
                return false;
        }
        return true;
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
