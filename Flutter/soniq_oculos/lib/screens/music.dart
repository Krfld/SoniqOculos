import '../imports.dart';

class Music extends StatefulWidget {
  @override
  _MusicState createState() => _MusicState();
}

class _MusicState extends State<Music> {
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
        bt.disconnect();
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
                          for (int i = 0; i < 3; i++)
                            if (i != data.devices)
                              Button(
                                text: 'Switch to\n${data.deviceName[i]}',
                                padding: 24,
                                margin: 8,
                                function: !app.processing ? () => data.switchDevices(i) : null,
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
                        text: data.mode != -1 ? 'Switch to Record' : 'Switching...',
                        padding: 24,
                        border: 8,
                        function: !app.processing ? () => data.changeMode() : null,
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
