import '../imports.dart';

class Record extends StatefulWidget {
  @override
  _RecordState createState() => _RecordState();
}

class _RecordState extends State<Record> {
  @override
  void initState() {
    super.initState();
    app.msg('InitState', context: context);

    app.context = app.msg(context, context: context);
  }

  @override
  Widget build(BuildContext context) {
    return WillPopScope(
      onWillPop: () {
        /*bt.context = null;
        bt.disconnect();
        Navigator.pop(context);*/
        return;
      },
      child: SafeArea(
        child: Scaffold(
          body: StreamBuilder(
              stream: app.stream,
              builder: (context, snapshot) {
                return Center(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                    children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                        children: [
                          Button(
                            text: data.record == 0 ? 'Start Recording' : 'Stop Recording',
                            padding: 24,
                            margin: 8,
                            function: !app.processing ? data.toggleRecording : null,
                          ),
                          Button(
                            text: data.playback == 0 ? 'Start Playback' : 'Stop Playback',
                            padding: 24,
                            margin: 8,
                            function: !app.processing ? data.togglePlayback : null,
                          ),
                        ],
                      ),
                      Separator(),
                      Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          EqualizerSliders(enable: !app.processing),
                          VolumeSlider(enable: !app.processing),
                        ],
                      ),
                      Separator(),
                      Button(
                        text: data.mode != -1 ? 'Switch to Music' : 'Switching...',
                        padding: 24,
                        border: 8,
                        function: !app.processing ? () => Navigator.pushReplacementNamed(context, 'Music') : null,
                      ),
                    ],
                  ),
                );
              }),
        ),
      ),
    );
  }
}
