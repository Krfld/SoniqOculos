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
                      Text('Music'),
                      Expanded(flex: 1, child: Center(child: VolumeSlider(enable: !data.processing))),
                      Expanded(
                        flex: 2,
                        child: Center(
                          child: Button(
                            text: 'Switch to Record',
                            padding: 24,
                            border: 8,
                            function: !data.processing ? () => Navigator.pushReplacementNamed(context, 'Record') : null,
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
