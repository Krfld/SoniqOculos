import './imports.dart';

final _App app = _App();

class _App {
  final String tag = '+';
  int _debugID = 1; // if 0, debug won't work
  dynamic msg(var msg, {BuildContext context, String prefix = 'DEBUG'}) {
    context == null && _debugID > 0
        ? print('$tag[${prefix.toUpperCase()} (${this._debugID++})] $msg')
        : print('$tag{${context.widget}} $msg');
    return msg;
  }
}
