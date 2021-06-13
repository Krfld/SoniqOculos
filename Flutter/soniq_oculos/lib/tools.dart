import './imports.dart';

final _App app = _App();

class _App {
  ///
  /// Volume
  ///
  int _volume = 50;
  int get volume => this._volume;
  set volume(int volume) => this._volume = volume;

  ///
  /// Context
  ///
  BuildContext _context;
  BuildContext get context => this._context;
  set context(BuildContext context) => this._context = context;

  ///
  /// Navigation
  ///
  void pop() {
    if (this._context != null) {
      Navigator.pop(context);
      this._context = null;
    }
  }

  ///
  /// Debug
  ///
  final String tag = '+';
  int _debugID = 1; // if 0, debug won't work
  dynamic msg(var msg, {BuildContext context, String prefix = 'DEBUG'}) {
    context == null && _debugID > 0
        ? print('$tag[${prefix.toUpperCase()} (${this._debugID++})] $msg')
        : print('$tag{${context.widget}} $msg');
    return msg;
  }
}

///
/// Loading
///
class Loading extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(body: Center(child: SpinKitChasingDots(color: Colors.teal)));
  }
}

///
/// Button
///
class Button extends StatelessWidget {
  final String text;
  final Function function;

  final bool enable;
  final Duration duration;

  final double padding;
  final double margin;
  final double border;

  Button({
    @required this.text,
    @required this.function,
    this.enable = true,
    this.duration,
    this.padding = 16,
    this.margin = 0,
    this.border = 32,
  });

  @override
  Widget build(BuildContext context) {
    return AnimatedOpacity(
      duration: this.duration ?? Duration(seconds: 0),
      opacity: this.enable ? 1 : 0,
      child: Padding(
        padding: EdgeInsets.all(this.margin),
        child: OutlineButton(
          //splashColor: Colors.teal,
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(this.border)),
          padding: EdgeInsets.all(this.padding),
          onPressed: this.enable ? this.function : null,
          child: Text(this.text, textAlign: TextAlign.center),
        ),
      ),
    );
  }
}

///
/// Volume Slider
///
class VolumeSlider extends StatefulWidget {
  @override
  _VolumeSliderState createState() => _VolumeSliderState();
}

class _VolumeSliderState extends State<VolumeSlider> {
  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        Text('Volume'),
        Padding(
          padding: EdgeInsets.symmetric(horizontal: 32),
          child: Slider.adaptive(
            min: 0,
            max: 100,
            divisions: 10,
            value: app.volume.toDouble(),
            label: '${app.volume}', //* Label doesn't work without divisions
            onChanged: (value) => setState(() => app.volume = value.toInt()),
            onChangeEnd: (value) => bt.sendCmd(app.volume),
          ),
        ),
      ],
    );
  }
}
