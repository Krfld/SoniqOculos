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
              stream: data.stream,
              builder: (context, snapshot) {
                return Center(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                    children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                        children: [
                          Button(
                            text: data.record == 0
                                ? 'Start Recording'
                                : data.record == 1
                                    ? 'Stop Recording'
                                    : 'Loading...',
                            padding: 24,
                            margin: 8,
                            function: !data.processing ? data.toggleRecording : null,
                          ),
                          Button(
                            text: data.playback == 0
                                ? 'Start Playback'
                                : data.playback == 1
                                    ? 'Stop Playback'
                                    : 'Loading...',
                            padding: 24,
                            margin: 8,
                            function: !data.processing ? data.togglePlayback : null,
                          ),
                        ],
                      ),
                      Separator(),
                      Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          EqualizerSliders(enable: !data.processing),
                          VolumeSlider(enable: !data.processing),
                        ],
                      ),
                      Separator(),
                      Button(
                        text: data.mode != -1 ? 'Switch to Music' : 'Switching...',
                        padding: 24,
                        border: 8,
                        function: !data.processing ? () => Navigator.pushReplacementNamed(context, 'Music') : null,
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
