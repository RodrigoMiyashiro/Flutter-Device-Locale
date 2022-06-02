import 'dart:ui';

import 'package:flutter_device_locale/src/platform.dart';

class DeviceLocale {
  static Future<Locale> getCurrentLocale() async {
    final deviceLocales = await getPreferredLocales();

    return deviceLocales.first;
  }

  static Future<List<Locale>> getPreferredLocales() async {
    final List<String> deviceLocales =
        await FlutterDeviceLocalePlatform.instance.deviceLocales();

    return deviceLocales.map((locale) => _localeFromString(locale)).toList();
  }

  static Locale _localeFromString(String code) {
    var separator = code.contains('_')
        ? '_'
        : code.contains('-')
            ? '-'
            : null;

    if (separator != null) {
      var parts = code.split(RegExp(separator));
      return Locale(parts[0], parts[1]);
    }

    return Locale(code);
  }
}
