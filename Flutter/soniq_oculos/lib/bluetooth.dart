import './imports.dart';

final String deviceAddress = '10:52:1C:67:9C:EA'; //* ESP Address

final _Bluetooth bt = _Bluetooth();

class _Bluetooth {
  FlutterBluetoothSerial bluetooth = FlutterBluetoothSerial.instance;

  Stream bluetoothStateStream = FlutterBluetoothSerial.instance.onStateChanged();
  //Stream _bluetoothInputStream;

  BluetoothState _bluetoothState = BluetoothState.UNKNOWN;
  bool get isBluetoothOn => this._bluetoothState.isEnabled;

  BluetoothBondState _bluetoothBondState = BluetoothBondState.unknown;
  bool get isDevicePaired => this._bluetoothBondState.isBonded;

  BluetoothConnection _bluetoothConnection;
  bool get isDeviceConnected => this._bluetoothConnection?.isConnected ?? false;

  bool _ready = false;

  void sendCmd(var cmd) {
    if (!isDeviceConnected) return;

    this._bluetoothConnection.output.add(ascii.encode(app.msg(cmd.toString())));
  }

  void getCmd(String msg) {
    app.msg(msg, prefix: 'Input');

    if (msg.compareTo('ON') == 0) this._ready = true;

    if (msg.compareTo('OK') == 0) {
      data.gotOK();
      return;
    }

    List characters = app.msg(msg.split(' '));
    switch (characters[0]) {
      case 'm': // Mode
        app.msg(int.parse(characters[1]));
        break;

      case 'v': // Volume
        break;

      case 'd': // Devices
        break;

      case 'e': // Equalizer
        break;

      case 's': // SD card
        break;

      case 'b': // Bone Conductors
        break;

      default:
        break;
    }
  }

  void setup() {
    bluetoothStateStream.listen((state) {
      this._bluetoothState = state;

      if (state == BluetoothState.STATE_OFF) app.pop();
    });
  }

  void disconnect() {
    this._bluetoothConnection.dispose();
  }

  Future connect() async {
    ///
    /// Bluetooth
    ///

    try {
      await bluetooth.requestEnable();
    } catch (e) {
      app.msg(e, prefix: 'Bluetooth');
    } finally {
      this._bluetoothState = await bluetooth.state;
    }

    if (isBluetoothOn)
      app.msg('Bluetooth is enabled', prefix: 'Bluetooth');
    else {
      app.msg('Bluetooth failed to enable', prefix: 'Bluetooth');
      return false;
    }

    ///
    /// Pair
    ///

    try {
      await bluetooth.bondDeviceAtAddress(deviceAddress);
    } catch (e) {
      app.msg(e, prefix: 'Pair');
    } finally {
      this._bluetoothBondState = await bluetooth.getBondStateForAddress(deviceAddress);
    }

    if (isDevicePaired)
      app.msg('Device is paired', prefix: 'Pair');
    else {
      app.msg('Device failed to pair', prefix: 'Pair');
      return false;
    }

    ///
    /// Connection
    ///

    try {
      await BluetoothConnection.toAddress(deviceAddress)
          .then((connection) => this._bluetoothConnection = connection)
          .timeout(Duration(seconds: 5));
    } catch (e) {
      //app.msg(e, prefix: 'Connection');
    }

    if (isDeviceConnected)
      app.msg('Device is connected', prefix: 'Connection');
    else {
      app.msg('Device failed to connect', prefix: 'Connection');
      return false;
    }

    ///
    /// Setup input
    ///

    StreamSubscription input = this._bluetoothConnection.input.listen((msg) => getCmd(ascii.decode(msg)));

    input.onDone(() {
      app.msg('Disconnected', prefix: 'Input');
      app.pop();
    });

    await Future.doWhile(() => !this._ready).timeout(Duration(seconds: 5)); // Wait for setup message

    if (!this._ready) {
      input.cancel(); // Cancel if ready not received
      return false;
    }

    return true;
  }
}
