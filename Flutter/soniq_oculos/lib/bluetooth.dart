import './imports.dart';

final _Bluetooth bt = _Bluetooth();

class _Bluetooth {
  FlutterBluetoothSerial bluetooth = FlutterBluetoothSerial.instance;

  bool _isBluetoothActive = false;
  bool get isBluetoothActive => this._isBluetoothActive;

  Future setup() async {
    app.msg(await bluetooth.address, prefix: 'address');

    app.msg(await bluetooth.name, prefix: 'name');

    app.msg(await bluetooth.state, prefix: 'state');

    bluetooth.requestEnable();

    bluetooth.onStateChanged().listen((state) {
      app.msg(state, prefix: 'state');
      if (state == BluetoothState.STATE_ON) this._isBluetoothActive = true;
      if (state == BluetoothState.STATE_OFF) this._isBluetoothActive = false;
    });

    BluetoothConnection.toAddress('SoniqOculos');
  }
}
