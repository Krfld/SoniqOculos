import './imports.dart';

final Settings settings = Settings();

class Settings {
  int mode;
  int volume;

  int devices;
  List<int> equalizer;

  bool record;
  bool playback;
}
