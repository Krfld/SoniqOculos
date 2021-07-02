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
      routes: {
        'Home': (context) => Home(),
        'Music': (context) => Music(),
        'Record': (context) => Record(),
      },
      home: Record(),
    );
  }
}
