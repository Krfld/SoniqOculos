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
                      Expanded(
                        flex: 3,
                        child: Center(
                          child: Column(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: [
                              Button(
                                text: data.record == 0
                                    ? 'Start Recording'
                                    : data.record == 1
                                        ? 'Stop Recording'
                                        : 'Sending...',
                                padding: 24,
                                margin: 16,
                                function: !data.processing ? data.toggleRecording : null,
                              ),
                              Button(
                                text: data.playback == 0
                                    ? 'Start Playback'
                                    : data.playback == 1
                                        ? 'Stop Playback'
                                        : 'Sending...',
                                padding: 24,
                                margin: 16,
                                function: !data.processing ? data.togglePlayback : null,
                              ),
                            ],
                          ),
                        ),
                      ),
                      Expanded(flex: 1, child: Center(child: VolumeSlider(enable: !data.processing))),
                      Expanded(
                        flex: 2,
                        child: Center(
                          child: Button(
                            text: 'Switch to Music',
                            padding: 24,
                            border: 8,
                            function: !data.processing ? () => Navigator.pushReplacementNamed(context, 'Music') : null,
                          ),
                          /*child: FloatingActionButton.extended(
                          label: Text('Switch to Music'),
                          onPressed: () => Navigator.pushReplacementNamed(context, 'Music'),
                        ),*/
                        ),
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
