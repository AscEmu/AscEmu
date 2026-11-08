using System.Net.Security;
using System.Net.Sockets;

namespace AscEmu.Launcher.Services;

public sealed record BattleNetStatus(bool Online, string Text);

public static class BattleNetStatusService
{
    public static async Task<BattleNetStatus> CheckAsync(string portal, CancellationToken cancellationToken = default)
    {
        try
        {
            var (host, port) = ParsePortal(portal);
            using var client = new TcpClient();
            using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            timeout.CancelAfter(TimeSpan.FromSeconds(2.5));
            await client.ConnectAsync(host, port, timeout.Token);
            using var tls = new SslStream(client.GetStream(), false, (_, _, _, _) => true);
            await tls.AuthenticateAsClientAsync(host);
            return new(true, $"{host}:{port} online");
        }
        catch (Exception ex) { return new(false, ex is OperationCanceledException ? $"{portal.Trim()} timeout" : $"{portal.Trim()} offline ({ex.GetBaseException().Message})"); }
    }

    public static (string Host, int Port) ParsePortal(string portal)
    {
        var value = portal.Trim();
        var split = value.LastIndexOf(':');
        return split > 0 && int.TryParse(value[(split + 1)..], out var port) ? (value[..split], port) : (value, 1119);
    }
}
