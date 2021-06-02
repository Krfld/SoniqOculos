import './imports.dart';

void main() => runApp(SoniqOculos());

class SoniqOculos extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'SoniqOculos',
      theme: ThemeData(
        primarySwatch: Colors.teal,
        visualDensity: VisualDensity.adaptivePlatformDensity,
        brightness: Brightness.dark,
      ),
      home: FutureBuilder(
        future: bt.setup(),
        builder: (context, setup) {
          if (setup.connectionState == ConnectionState.waiting)
            return Scaffold(body: Center(child: Text('Loading...')));

          if (!bt.isBluetoothOn) return Home();
          return Scaffold(body: Center(child: Text("SoniqOculos")));
        },
      ),
    );
  }
}
