import 'dart:convert';

import './imports.dart';

final _Bluetooth bt = _Bluetooth();

final String espAddress = '10:52:1C:67:9C:EA';

class _Bluetooth {
  FlutterBluetoothSerial bluetooth = FlutterBluetoothSerial.instance;
  BluetoothConnection bluetoothConnection;

  BluetoothState _bluetoothState = BluetoothState.UNKNOWN;
  bool get isBluetoothOn => this._bluetoothState == BluetoothState.STATE_ON;

  Future setup() async {
    app.msg(await bluetooth.address, prefix: 'address');

    app.msg(await bluetooth.name, prefix: 'name');

    bluetooth.onStateChanged().listen((state) {
      this._bluetoothState = app.msg(state, prefix: 'state');
    });

    await bluetooth.requestEnable();

    this._bluetoothState = app.msg(await bluetooth.state, prefix: 'state');

    if (await bluetooth.getBondStateForAddress(espAddress) != BluetoothBondState.bonded)
      app.msg(await bluetooth.bondDeviceAtAddress(espAddress), prefix: 'bond');

    bluetoothConnection = await BluetoothConnection.toAddress(espAddress);

    app.msg(bluetoothConnection.isConnected, prefix: 'isConnected');

    bluetoothConnection.input.listen((data) => app.msg(ascii.decode(data)));

    bluetoothConnection.output.add(ascii.encode('Xiu'));

    bluetoothConnection.dispose();
  }
}
