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
                            text: 'Loading...',
                            padding: 24,
                            margin: 8,
                            function: !data.processing ? () => null : null,
                          ),
                          Button(
                            text: 'Loading...',
                            padding: 24,
                            margin: 8,
                            function: !data.processing ? () => null : null,
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
                        text: data.mode != -1 ? 'Switch to Record' : 'Switching...',
                        padding: 24,
                        border: 8,
                        function: !data.processing ? () => Navigator.pushReplacementNamed(context, 'Record') : null,
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
