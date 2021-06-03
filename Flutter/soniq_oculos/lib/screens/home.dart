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

    if (await bt.connect()) await Navigator.pushNamed(context, 'Music');

    setState(() => this._connecting = false);
  }

  @override
  Widget build(BuildContext context) {
    var textTheme = Theme.of(context).textTheme;

    return StreamBuilder(
      stream: bt.bluetoothStateStream,
      builder: (context, state) {
        return Scaffold(
          body: Column(
            mainAxisAlignment: MainAxisAlignment.spaceEvenly,
            children: [
              Expanded(
                child: Center(
                  child: Text(
                    'SoniqOculos',
                    style: textTheme.headline3,
                  ),
                ),
              ),
              Expanded(
                child: Center(
                  child: this._connecting
                      ? SpinKitChasingDots(color: Colors.teal)
                      : Button(
                          text: 'Connect Device',
                          function: () => this._connect(),
                        ),
                ),
              ),
            ],
          ),
        );
      },
    );
  }
}
