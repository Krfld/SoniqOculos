import './imports.dart';

final Data data = Data();

class Data {
  ///
  /// Mode
  ///

  int _mode = 0;
  int get mode => this._mode;
  set mode(int mode) => this._mode = mode;

  void changeMode() => bt.sendCmd('m ${1 - mode}');

  ///
  /// Volume
  ///

  int _volume = 50;
  int get volume => this._volume;
  set volume(int volume) => this._volume = volume;

  void changeVolume(int volume) => bt.sendCmd('v $volume');

  ///
  /// Devices
  ///

  int _devices = 0;
  int get devices => this._devices;
  set devices(int devices) => this._devices = devices;

  ///
  /// Equalizer
  ///

  final int equalizerGain = 3; // dB

  int _bass = 0;
  int _mid = 0;
  int _treble = 0;

  int get bass => this._bass;
  set bass(int bass) => this._bass = bass;

  int get mid => this._mid;
  set mid(int mid) => this._mid = mid;

  int get treble => this._treble;
  set treble(int treble) => this._treble = treble;

  void updateEqualizer({int bass, int mid, int treble}) {
    bass = bass ?? this._bass;
    mid = mid ?? this._mid;
    treble = treble ?? this._treble;

    if (bass == this._bass && mid == this._mid && treble == this._treble) return; // Return if no change is made

    bt.sendCmd('e $bass $mid $treble');
  }

  ///
  /// Record
  ///

  int _record = 0;
  int get record => this._record;
  set record(int record) => this._record = record;

  void toggleRecording() => bt.sendCmd('r ${1 - record}');

  ///
  /// Playback
  ///

  int _playback = 0;
  int get playback => this._playback;
  set playback(int playback) => this._playback = playback;

  void togglePlayback() => bt.sendCmd('p ${1 - playback}');
}
