using System.Diagnostics;
using System.Media;
using System.Numerics;
using System.Runtime.InteropServices;
using AscEmu.Launcher.Services;

namespace AscEmu.Launcher.Patching;

// Writes the AscEmu public key into the region-key table of the client and keeps scanning: the client builds the
// table on its heap when it connects to the world server and builds a new one for the next connect.
public sealed class RegionKeyMonitor : IDisposable
{
    const int RecordSize = 0x28;
    const int RecordWords = RecordSize / sizeof(uint);
    const int KeyOffset = 0x04;
    const int KeySize = 32;
    const int FlagOffset = 0x24;
    const int MinRecordCount = 8;
    const int MaxRecordCount = 32;
    const int ServerKeyId = 8;
    const int MinDistinctKeyBytes = 20;
    const int MaxLoggedLookAlikes = 32;
    const long MaximumRegionSize = 64L * 1024L * 1024L;

    static readonly BigInteger FieldPrime = BigInteger.Pow(2, 255) - 19;
    static readonly BigInteger CurveD = BigInteger.Parse("37095705934669439343138083508754565189542113879843219016388785533085940283555");

    readonly nint _processHandle;
    readonly int _processId;
    readonly bool _playSound;
    readonly CancellationTokenSource _cts = new();
    readonly Dictionary<nint, int> _tables = new();
    readonly HashSet<nint> _lookAlikes = new();
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
        Report("Attached to the client; waiting for the region-key table...");

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
                var writtenKeys = PatchTables(memory);
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

    // scans the heap of the client and patches every key table where it is found; number of keys that were replaced
    int PatchTables(ProcessMemory memory)
    {
        var writtenKeys = 0;
        foreach (var region in memory.ReadableRegions())
        {
            // the table is built on the heap; image and mapped memory only hold look-alikes
            if (region.Type != NativeMethods.MemPrivate || !IsWritable(region.Protect))
                continue;
            if (region.Size < MinRecordCount * RecordSize || region.Size > MaximumRegionSize)
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

                var candidate = bytes[(index * sizeof(uint))..];
                var count = CountRecords(candidate);
                if (count < MinRecordCount)
                    continue;

                var tableBase = region.Base + index * sizeof(uint);
                var layout = DescribeRecords(candidate, count);
                if (IsKeyTable(layout))
                    writtenKeys += PatchTable(memory, tableBase, count);
                else if (_lookAlikes.Count < MaxLoggedLookAlikes && _lookAlikes.Add(tableBase))
                    LauncherLog.Write($"Skipped ids 1..{count} at 0x{tableBase.ToInt64():X}, no key table ({layout})");

                index += count * RecordWords - 1;
            }
        }
        return writtenKeys;
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

    // one letter per record: K public key, 0 empty, - other data
    static string DescribeRecords(ReadOnlySpan<byte> bytes, int count)
    {
        Span<char> layout = stackalloc char[count];
        for (var index = 0; index < count; ++index)
        {
            var key = bytes.Slice(index * RecordSize + KeyOffset, KeySize);
            layout[index] = IsPublicKey(key) ? 'K' : key.IndexOfAnyExcept((byte)0) < 0 ? '0' : '-';
        }
        return new string(layout);
    }

    // only keys and empty records, and a key in the record the server signs for
    static bool IsKeyTable(string layout) => !layout.Contains('-') && layout[ServerKeyId - 1] == 'K';

    // 32 random looking bytes that decode to a point of the Ed25519 curve
    static bool IsPublicKey(ReadOnlySpan<byte> key)
    {
        Span<bool> seen = stackalloc bool[256];
        seen.Clear();
        var distinct = 0;
        foreach (var value in key)
        {
            if (seen[value])
                continue;
            seen[value] = true;
            ++distinct;
        }
        if (distinct < MinDistinctKeyBytes)
            return false;

        Span<byte> encoded = stackalloc byte[KeySize];
        key.CopyTo(encoded);
        encoded[KeySize - 1] &= 0x7F;
        var y = new BigInteger(encoded, isUnsigned: true);
        if (y >= FieldPrime)
            return false;

        // x^2 = (y^2 - 1) / (d y^2 + 1) has a root
        var ySquared = y * y % FieldPrime;
        var numerator = (ySquared + FieldPrime - 1) % FieldPrime;
        var denominator = (CurveD * ySquared + 1) % FieldPrime;
        var residue = BigInteger.ModPow(numerator * denominator % FieldPrime, (FieldPrime - 1) / 2, FieldPrime);
        return residue.IsZero || residue.IsOne;
    }

    // number of keys that were replaced; the table is read again first, the memory may have changed since the scan
    int PatchTable(ProcessMemory memory, nint tableBase, int count)
    {
        var table = new byte[count * RecordSize];
        if (!memory.Read(tableBase, table) || CountRecords(table) != count)
            return 0;

        var layout = DescribeRecords(table, count);
        if (!IsKeyTable(layout))
            return 0;

        if (!_tables.TryGetValue(tableBase, out var known) || known != count)
        {
            _tables[tableBase] = count;
            LauncherLog.Write($"Region-key table with ids 1..{count} at 0x{tableBase.ToInt64():X} ({layout})");
        }

        var written = 0;
        for (var index = 0; index < count; ++index)
        {
            var keyOffset = index * RecordSize + KeyOffset;
            if (table.AsSpan(keyOffset, KeySize).SequenceEqual(ForeverPatchProfile.AscEmuEd25519PublicKey))
                continue;

            if (memory.Write(tableBase + keyOffset, ForeverPatchProfile.AscEmuEd25519PublicKey))
                ++written;
        }

        if (written > 0)
            LauncherLog.Write($"Wrote {written} of {count} keys at 0x{tableBase.ToInt64():X}");
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
