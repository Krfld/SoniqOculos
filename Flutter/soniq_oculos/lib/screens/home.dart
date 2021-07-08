import '../imports.dart';

class Home extends StatefulWidget {
  @override
  _HomeState createState() => _HomeState();
}

class _HomeState extends State<Home> {
  bool _connecting = false;

  @override
  void initState() {
    super.initState();
    app.msg('InitState', context: context);

    bt.setup();
    this._connect();
  }

  Future _connect() async {
    setState(() => this._connecting = true);
    //app.process(() => null);

    if (await bt.connect()) await Navigator.pushNamed(context, data.mode == 0 ? 'Music' : 'Record');

    //app.done();

    setState(() => this._connecting = false);
  }

  @override
  Widget build(BuildContext context) {
    var textTheme = Theme.of(context).textTheme;

    return SafeArea(
      child: StreamBuilder(
        stream: bt.bluetoothStateStream,
        builder: (context, state) {
          return Scaffold(
            body: Column(
              children: [
                Expanded(child: Center(child: Text('SoniqOculos', style: textTheme.headline3))),
                Expanded(
                  child: Center(
                    child: !this._connecting
                        ? Button(
                            text: 'Connect Device',
                            padding: 32,
                            border: 64,
                            function: () => this._connect(),
                          )
                        : SpinKitChasingDots(color: Colors.teal),
                  ),
                ),
              ],
            ),
          );
        },
      ),
    );
  }
}
