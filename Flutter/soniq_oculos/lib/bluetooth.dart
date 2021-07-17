import 'dart:typed_data';

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

  ///
  /// Setup
  ///

  void setup() {
    bluetoothStateStream.listen((state) {
      this._bluetoothState = state;

      if (state == BluetoothState.STATE_OFF) {
        this._ready = false; // If bluetooth turned off
        disconnect();
        app.pop();
      }
    });
  }

  ///
  /// Connect
  ///

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

    if (!this._ready)
      this._bluetoothConnection.input.listen((msg) => getCmd(msg)).onDone(() {
        app.msg('Disconnected', prefix: 'Connection'); // If connection closed
        this._ready = false;
        disconnect();
        app.pop();
        app.done();
      });

    await Future.delayed(Duration(seconds: 2)); // Wait for setup message

    return this._ready;
  }

  ///
  /// Disconnect
  ///

  void disconnect() {
    this._bluetoothConnection?.dispose();
  }

  ///
  /// Send
  ///

  void sendCmd(var cmd) {
    if (!isDeviceConnected || app.processing) return;

    data.recordFail = false; // Reset record state

    try {
      this._bluetoothConnection.output.add(ascii.encode(app.msg(cmd.toString(), prefix: 'Output'))); // Send message
    } catch (e) {
      app.msg(e, prefix: 'Error');
    }
  }

  ///
  /// Receive
  ///

  void getCmd(Uint8List msg) {
    String cmd = '';
    try {
      cmd = ascii.decode(msg); // Decode message
    } catch (e) {
      app.msg(e, prefix: 'Error');
    }

    List words = cmd.split(' '); // Split message by spaces
    app.msg(words, prefix: 'Input');

    switch (words[0]) {
      case 'SETUP':
        data.mode = int.tryParse(words[2]) ?? 0; // Mode

        data.volume = (int.tryParse(words[4]) ?? 5) * 10; // Volume
        data.devices = int.tryParse(words[6]) ?? 0; // Devices
        data.bass = int.tryParse(words[8]) ?? 0; // Bass
        data.mid = int.tryParse(words[10]) ?? 0; // Mid
        data.treble = int.tryParse(words[12]) ?? 0; // Treble

        data.record = int.tryParse(words[14]) ?? 0; // SD
        data.playback = int.tryParse(words[16]) ?? 0; // BCD
        this._ready = true;
        break;

      case 'm': // Mode
        data.mode = int.tryParse(words[1]) ?? 0;
        app.push(data.mode == 0 ? 'Music' : 'Record');
        break;

      case 'v': // Volume
        data.volume = (int.tryParse(words[1]) ?? 5) * 10;
        break;

      case 'd': // Devices
        data.devices = int.tryParse(words[1]) ?? 0;
        break;

      case 'eb': // Bass
        data.bass = int.tryParse(words[1]) ?? 0;
        break;

      case 'em': // Mid
        data.mid = int.tryParse(words[1]) ?? 0;
        break;

      case 'et': // Treble
        data.treble = int.tryParse(words[1]) ?? 0;
        break;

      case 'r': // SD card
        int value = int.tryParse(words[1]) ?? 0;
        if (value == data.record && data.record != 0) data.recordFail = true; // Error starting recording
        data.record = value;
        break;

      case 'p': // Bone Conductors
        data.playback = int.tryParse(words[1]) ?? 0;
        break;

      default:
        app.msg('Unknown message ($cmd)', prefix: 'Command');
        disconnect();
        break;
    }

    app.done();
  }
}
