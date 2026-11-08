using System.Windows;
using System.Windows.Media;
using AscEmu.Launcher.Core;
using AscEmu.Launcher.Models;
using AscEmu.Launcher.Services;

namespace AscEmu.Launcher;

public partial class MainWindow : Window
{
    readonly LauncherSettings _settings;
    readonly CancellationTokenSource _statusCancellation = new();
    RunningGame? _runningGame;
    bool _battleNetOnline;
    bool _clientReady;
    bool _launchInProgress;

    public MainWindow()
    {
        InitializeComponent();
        _settings = SettingsService.Load();
        Loaded += async (_, _) =>
        {
            RefreshClient();
            await RefreshServerAsync();
            _ = MonitorBattleNetAsync(_statusCancellation.Token);
        };
        Closed += (_, _) =>
        {
            _statusCancellation.Cancel();
            _statusCancellation.Dispose();
            _runningGame?.Dispose();
        };
    }

    async Task MonitorBattleNetAsync(CancellationToken cancellationToken)
    {
        using var timer = new PeriodicTimer(TimeSpan.FromSeconds(10));
        try
        {
            while (await timer.WaitForNextTickAsync(cancellationToken))
                await RefreshServerAsync(cancellationToken);
        }
        catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
        {
        }
    }

    async Task RefreshServerAsync(CancellationToken cancellationToken = default)
    {
        var status = await BattleNetStatusService.CheckAsync(_settings.Portal, cancellationToken);
        if (cancellationToken.IsCancellationRequested)
            return;

        _battleNetOnline = status.Online;
        ServerText.Text = "Battle.net: " + status.Text;
        ServerText.ToolTip = string.IsNullOrEmpty(SettingsService.LoadedPath) ? "default settings" : SettingsService.LoadedPath;
        ServerText.Foreground = status.Online ? (Brush)FindResource("SuccessBrush") : (Brush)FindResource("DangerBrush");
        UpdatePlayButtonState();
    }

    void RefreshClient()
    {
        var client = ClientLocator.Locate();
        if (client == null)
        {
            _clientReady = false;
            BuildText.Text = $"{ClientLocator.ExpectedBinaryNames} not found";
            PatchDetailsText.Text = $"Place AscEmu Launcher.exe next to {ClientLocator.ExpectedBinaryNames}";
            UpdatePlayButtonState();
            return;
        }

        _clientReady = BuildProfiles.IsSupported(client.Build);
        BuildText.Text = BuildProfiles.GetLabel(client.Build);
        PatchDetailsText.Text = _clientReady ? $"Supported Forever build · {BuildProfiles.MinSupportedBuild}–{BuildProfiles.MaxSupportedBuild}" : $"Unsupported build · supported {BuildProfiles.MinSupportedBuild}–{BuildProfiles.MaxSupportedBuild}";
        UpdatePlayButtonState();
    }

    void UpdatePlayButtonState() => PlayButton.IsEnabled = _clientReady && _battleNetOnline && !_launchInProgress;

    void SetPatchStatus(string text) => Dispatcher.Invoke(() => PatchStatusText.Text = text);

    void Play_Click(object sender, RoutedEventArgs e)
    {
        if (!_battleNetOnline)
        {
            PatchStatusText.Text = "Battle.net unavailable";
            return;
        }

        var client = ClientLocator.Locate();
        if (client == null)
        {
            _clientReady = false;
            UpdatePlayButtonState();
            MessageBox.Show($"{ClientLocator.ExpectedBinaryNames} was not found next to AscEmu Launcher.exe.", "AscEmu Launcher", MessageBoxButton.OK, MessageBoxImage.Information);
            return;
        }

        if (!BuildProfiles.IsSupported(client.Build))
        {
            _clientReady = false;
            UpdatePlayButtonState();
            MessageBox.Show($"Unsupported Forever build {client.Build}. Supported builds are {BuildProfiles.MinSupportedBuild} through {BuildProfiles.MaxSupportedBuild}.", "AscEmu Launcher", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        _launchInProgress = true;
        UpdatePlayButtonState();
        try
        {
            _runningGame?.Dispose();
            _runningGame = GameLauncher.Launch(client, _settings, SetPatchStatus);
            PatchStatusText.Text = "Game launched - waiting for region keys";
        }
        catch (Exception ex)
        {
            PatchStatusText.Text = "Launch failed";
            MessageBox.Show(ex.Message, "AscEmu Launcher", MessageBoxButton.OK, MessageBoxImage.Error);
        }
        finally
        {
            _launchInProgress = false;
            UpdatePlayButtonState();
        }
    }
}
