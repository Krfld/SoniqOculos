import './imports.dart';

final _App app = _App();

class _App {
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
/// Separator
///

class Separator extends StatelessWidget {
  final double _indent = 50;

  @override
  Widget build(BuildContext context) {
    return Divider(
      indent: this._indent,
      endIndent: this._indent,
    );
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

  //final Color borderColor;

  Button({
    @required this.text,
    @required this.function,
    this.enable = true,
    this.duration,
    this.padding = 16,
    this.margin = 0,
    this.border = 32,
    //this.borderColor,
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
          //borderSide: BorderSide(color: this.borderColor == null ? Colors.grey : this.borderColor),
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
  final bool enable;

  VolumeSlider({this.enable = true});

  @override
  _VolumeSliderState createState() => _VolumeSliderState();
}

class _VolumeSliderState extends State<VolumeSlider> {
  int sliderValue = data.volume;

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        Text('Volume'),
        Padding(
          padding: EdgeInsets.symmetric(horizontal: 32),
          child: Slider.adaptive(
            min: 0,
            max: 100,
            divisions: 10,
            value: sliderValue.toDouble(),
            label: '$sliderValue', //* Label doesn't work without divisions
            onChanged: this.widget.enable ? (value) => setState(() => sliderValue = value.toInt()) : null,
            onChangeEnd: (value) {
              if (sliderValue != data.volume) {
                data.volume = sliderValue;
                bt.sendCmd('v ${data.volume}');
              }
            },
          ),
        ),
      ],
    );
  }
}

///
/// Equalizer Sliders
///

class EqualizerSliders extends StatefulWidget {
  final bool enable;

  EqualizerSliders({this.enable = true});

  @override
  _EqualizerSlidersState createState() => _EqualizerSlidersState();
}

class _EqualizerSlidersState extends State<EqualizerSliders> {
  int sliderBassValue = data.bass;
  int sliderMidValue = data.mid;
  int sliderTrebleValue = data.treble;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: EdgeInsets.symmetric(horizontal: 32),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
        children: [
          Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text('Bass'),
              RotatedBox(
                quarterTurns: -1,
                child: Slider.adaptive(
                  min: -4,
                  max: 0,
                  divisions: 4,
                  value: sliderBassValue.toDouble(),
                  label: '${sliderBassValue + 2}', //* Label doesn't work without divisions
                  onChanged: this.widget.enable ? (value) => setState(() => sliderBassValue = value.toInt()) : null,
                  onChangeEnd: (value) {
                    if (sliderBassValue != data.bass) {
                      data.bass = sliderBassValue;
                      bt.sendCmd('e ${data.bass} ${data.mid} ${data.treble}');
                    }
                  },
                ),
              ),
            ],
          ),
          Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text('Mid'),
              RotatedBox(
                quarterTurns: -1,
                child: Slider.adaptive(
                  min: -4,
                  max: 0,
                  divisions: 4,
                  value: sliderMidValue.toDouble(),
                  label: '${sliderMidValue + 2}', //* Label doesn't work without divisions
                  onChanged: this.widget.enable ? (value) => setState(() => sliderMidValue = value.toInt()) : null,
                  onChangeEnd: (value) {
                    if (sliderMidValue != data.mid) {
                      data.mid = sliderMidValue;
                      bt.sendCmd('e ${data.bass} ${data.mid} ${data.treble}');
                    }
                  },
                ),
              ),
            ],
          ),
          Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text('Treble'),
              RotatedBox(
                quarterTurns: -1,
                child: Slider.adaptive(
                  min: -4,
                  max: 0,
                  divisions: 4,
                  value: sliderTrebleValue.toDouble(),
                  label: '${sliderTrebleValue + 2}', //* Label doesn't work without divisions
                  onChanged: this.widget.enable ? (value) => setState(() => sliderTrebleValue = value.toInt()) : null,
                  onChangeEnd: (value) {
                    if (sliderTrebleValue != data.treble) {
                      data.treble = sliderTrebleValue;
                      bt.sendCmd('e ${data.bass} ${data.mid} ${data.treble}');
                    }
                  },
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }
}
