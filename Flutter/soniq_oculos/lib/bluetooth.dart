import './imports.dart';

final _Bluetooth bt = _Bluetooth();

class _Bluetooth {
  FlutterBluetoothSerial bluetooth = FlutterBluetoothSerial.instance;

  BluetoothState _bluetoothState = BluetoothState.UNKNOWN;
  bool get isBluetoothOn => this._bluetoothState == BluetoothState.STATE_ON;

  Future setup() async {
    app.msg(await bluetooth.address, prefix: 'address');

    app.msg(await bluetooth.name, prefix: 'name');

    this._bluetoothState = app.msg(await bluetooth.state, prefix: 'state');

    bluetooth.requestEnable();

    bluetooth.onStateChanged().listen((state) {
      this._bluetoothState = app.msg(state, prefix: 'state');
    });
  }
}
