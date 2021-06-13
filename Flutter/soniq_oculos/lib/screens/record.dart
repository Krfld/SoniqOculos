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
                Text('Record'),
                VolumeSlider(),
                FloatingActionButton.extended(
                  label: Text('Switch to Music'),
                  onPressed: () => Navigator.pushReplacementNamed(context, 'Music'),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
