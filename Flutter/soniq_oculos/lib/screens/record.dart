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
          body: Center(
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
                          text: 'Start Recording',
                          borderColor: Colors.red,
                          padding: 32,
                          margin: 16,
                          function: () => null,
                        ),
                        Button(
                          text: 'Start Playback',
                          borderColor: Colors.red,
                          padding: 32,
                          margin: 16,
                          function: () => null,
                        ),
                      ],
                    ),
                  ),
                ),
                Expanded(flex: 1, child: Center(child: VolumeSlider())),
                Expanded(
                  flex: 2,
                  child: Center(
                    child: FloatingActionButton.extended(
                      label: Text('Switch to Music'),
                      onPressed: () => Navigator.pushReplacementNamed(context, 'Music'),
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
