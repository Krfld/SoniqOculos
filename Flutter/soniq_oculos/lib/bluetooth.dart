import './imports.dart';

final String deviceAddress = '10:52:1C:67:9C:EA'; //* ESP Address

final _Bluetooth bt = _Bluetooth();

class _Bluetooth {
  BuildContext _context;
  BuildContext get context => this._context;
  set context(BuildContext context) => this._context = context;

  FlutterBluetoothSerial bluetooth = FlutterBluetoothSerial.instance;

  Stream bluetoothStateStream = FlutterBluetoothSerial.instance.onStateChanged();
  Stream _bluetoothInputStream;

  BluetoothState _bluetoothState = BluetoothState.UNKNOWN;
  bool get isBluetoothOn => this._bluetoothState.isEnabled;

  BluetoothBondState _bluetoothBondState = BluetoothBondState.unknown;
  bool get isDevicePaired => this._bluetoothBondState.isBonded;

  BluetoothConnection _bluetoothConnection;
  bool get isDeviceConnected => this._bluetoothConnection?.isConnected ?? false;

  void setup() {
    bluetoothStateStream.listen((state) {
      this._bluetoothState = state;

      if (state == BluetoothState.STATE_OFF && this._context != null) Navigator.pop(context);
    });

    return;

    this._bluetoothConnection.output.add(ascii.encode('Xiu'));
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
      this._bluetoothConnection = null;
    }

    if (isDeviceConnected)
      app.msg('Device is connected', prefix: 'Connection');
    else {
      app.msg('Device failed to connect', prefix: 'Connection');
      return false;
    }

    this._bluetoothInputStream = this._bluetoothConnection.input;
    this._bluetoothInputStream.listen((data) => app.msg(ascii.decode(data), prefix: 'Input Stream')).onDone(() {
      app.msg('Disconnected', prefix: 'Input Stream');
      this._bluetoothConnection = null;

      if (this._context != null) Navigator.pop(context);
    });

    return true;
  }
}
