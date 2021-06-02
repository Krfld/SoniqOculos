import '../imports.dart';

class Home extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: OutlinedButton(
          child: Text('Turn Bluetooth On'),
          onPressed: () => null,
        ),
      ),
    );
  }
}
