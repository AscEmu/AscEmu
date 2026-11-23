using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.RegularExpressions;

namespace AscEmu.Launcher;

public static class SystemTrust
{
    private static readonly Regex PemCertRegex = new(
        @"-----BEGIN CERTIFICATE-----[\s\S]+?-----END CERTIFICATE-----",
        RegexOptions.Compiled);

    public static void Ensure(string host, string serverAddress, int port, string? caCertificatePath, Action<string> status)
    {
        if (string.IsNullOrWhiteSpace(host))
            return;

        var needsHosts = !IPAddress.TryParse(host, out _) && !ResolvesTo(host, serverAddress);
        var hasRootCa = TryExtractRootCaPem(caCertificatePath, out var rootCaPem);
        var needsCa = hasRootCa && !IsCaTrusted(rootCaPem);

        if (!needsHosts && !needsCa)
            return;

        var hostsPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "drivers", "etc", "hosts");
        var script = new StringBuilder();
        string? tempCaPath = null;

        try
        {
            if (needsHosts)
            {
                var safeHost = host.Replace("'", "''");
                var safeIp = serverAddress.Replace("'", "''");
                var safeHostsPath = hostsPath.Replace("'", "''");
                var escapedRegexHost = Regex.Escape(host).Replace("'", "''");

                script.AppendLine($"$line = '{safeIp} {safeHost}';");
                script.AppendLine($"$content = Get-Content -Path '{safeHostsPath}' -ErrorAction SilentlyContinue;");
                script.AppendLine($"if (-not ($content -match '(^|\\s){escapedRegexHost}(\\s|$)')) {{ Add-Content -Path '{safeHostsPath}' -Value \"`r`n$line\" -Encoding ASCII }};");
            }

            if (needsCa)
            {
                tempCaPath = Path.Combine(Path.GetTempPath(), $"ascemu_root_ca_{Guid.NewGuid():N}.pem");
                File.WriteAllText(tempCaPath, rootCaPem + Environment.NewLine, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));

                var safeTempCaPath = tempCaPath.Replace("'", "''");
                script.AppendLine($"certutil.exe -addstore -f 'Root' '{safeTempCaPath}' | Out-Null;");
                script.AppendLine("if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE };");
            }

            status("Requesting elevation for hosts / certificate setup...");
            var psi = new ProcessStartInfo
            {
                FileName = "powershell.exe",
                Arguments = $"-NoProfile -ExecutionPolicy Bypass -Command \"{script}\"",
                Verb = "runas",
                UseShellExecute = true,
                WindowStyle = ProcessWindowStyle.Hidden
            };

            using var proc = Process.Start(psi);
            proc?.WaitForExit();
            LauncherLog.Write($"SystemTrust updated (hosts={needsHosts}, ca={needsCa}, exitCode={proc?.ExitCode})");
        }
        catch (Win32Exception ex)
        {
            LauncherLog.Write($"SystemTrust elevation cancelled or failed: {ex.Message}");
            status("Hosts/CA setup skipped (UAC cancelled).");
        }
        catch (Exception ex)
        {
            LauncherLog.Write($"SystemTrust error: {ex.Message}");
            status($"Hosts/CA setup failed: {ex.Message}");
        }
        finally
        {
            if (tempCaPath is not null)
            {
                try { File.Delete(tempCaPath); } catch { }
            }
        }
    }

    private static bool TryExtractRootCaPem(string? pemPath, out string rootCaPem)
    {
        rootCaPem = string.Empty;
        if (string.IsNullOrWhiteSpace(pemPath) || !File.Exists(pemPath))
            return false;

        try
        {
            var content = File.ReadAllText(pemPath);
            var matches = PemCertRegex.Matches(content);
            if (matches.Count == 0)
                return false;

            rootCaPem = matches[^1].Value.Trim();
            return true;
        }
        catch
        {
            return false;
        }
    }

    private static bool ResolvesTo(string host, string expectedIp)
    {
        try
        {
            if (!IPAddress.TryParse(expectedIp, out var target))
                return false;

            var addresses = Dns.GetHostAddresses(host);
            return addresses.Any(a => a.Equals(target));
        }
        catch
        {
            return false;
        }
    }

    private static bool IsCaTrusted(string rootCaPem)
    {
        try
        {
            using var cert = X509Certificate2.CreateFromPem(rootCaPem);
            using var store = new X509Store(StoreName.Root, StoreLocation.LocalMachine);
            store.Open(OpenFlags.ReadOnly);
            return store.Certificates.Find(X509FindType.FindByThumbprint, cert.Thumbprint, false).Count > 0;
        }
        catch
        {
            return false;
        }
    }
}