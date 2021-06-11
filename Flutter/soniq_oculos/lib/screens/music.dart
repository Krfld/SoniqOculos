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

    bt.context = app.msg(context, context: context);
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
                Text('Music'),
                Button(
                  text: 'Record',
                  function: () => Navigator.pushReplacementNamed(context, 'Record'),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
