import './imports.dart';

final Data data = Data();

class Data {
  ///
  /// Mode
  ///

  int _mode = 0;
  int get mode => this._mode;
  set mode(int value) => this._mode = value;

  ///
  /// Volume
  ///

  int _volume = 50;
  int get volume => this._volume;
  set volume(int volume) => this._volume = volume;

  ///
  /// Devices
  ///

  int _devices = 0;

  ///
  /// Equalizer
  ///

  final int equalizerGain = 3; // dB

  int _bass = 0;
  int _mid = 0;
  int _treble = 0;

  int get bass => this._bass;
  set bass(int value) => this._bass = value;

  int get mid => this._mid;
  set mid(int value) => this._mid = value;

  int get treble => this._treble;
  set treble(int value) => this._treble = value;

  void updateEqualizer({int bass, int mid, int treble}) {
    bass = bass ?? this._bass;
    mid = mid ?? this._mid;
    treble = treble ?? this._treble;

    if (bass == this._bass && mid == this._mid && treble == this._treble) return; // Return if no change is made

    bt.sendCmd('e $bass $mid $treble');
    //app.process(() => bt.sendCmd('e $bass $mid $treble'));
  }

  ///
  /// Record
  ///

  int _record = 0;
  int get record => this._record;
  set record(int value) => this._record = value;

  void toggleRecording() => bt.sendCmd('r ${1 - record}'); //app.process(() => bt.sendCmd('r ${1 - record}'));

  ///
  /// Playback
  ///

  int _playback = 0;
  int get playback => this._playback;
  set playback(int value) => this._playback = value;

  void togglePlayback() => bt.sendCmd('r ${1 - playback}'); //app.process(() => bt.sendCmd('r ${1 - playback}'));
}
