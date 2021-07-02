import './imports.dart';

final Data data = Data();

class Data {
  // ignore: close_sinks
  final StreamController streamController = StreamController.broadcast();
  Stream get stream => this.streamController.stream;

  bool _processing = false;
  bool get processing => this._processing;

  void _process(Function function) {
    this._processing = true;
    function();
    this.streamController.sink.add(null); // Update screens
  }

  void gotOK() {
    this._processing = false;
    this.streamController.sink.add(null); // Update screens
  }

  ///
  /// Mode
  ///

  int mode = 0; // -1 when waiting for OK

  ///
  /// Volume
  ///

  int volume = 50; // -1 when waiting for OK

  ///
  /// Devices
  ///

  int devices = 0; // -1 when waiting for OK

  ///
  /// Equalizer
  ///

  List<int> equalizer = [-2, -2, -2]; // 1 when waiting for OK

  ///
  /// Record
  ///

  int _record = 0; // -1 when waiting for OK
  int get record => this._record;

  void updateRecord(int value) {
    this._record = value;
  }

  void toggleRecording() {
    if (this._processing || _record == -1) return;

    this._process(() {
      this._record == 0 ? this._record = 1 : this._record = 0;
      bt.sendCmd('r $_record');
      this._record = -1;
    });
  }

  ///
  /// Playback
  ///

  int _playback = 0; // -1 when waiting for OK
  int get playback => this._playback;

  void updatePlayback(int value) {
    this._playback = value;
  }

  void togglePlayback() {
    if (this._processing || _playback == -1) return;

    this._process(() {
      this._playback == 0 ? this._playback = 1 : this._playback = 0;
      bt.sendCmd('p $_playback');
      this._playback = -1;
    });
  }
}
