#include "include/flutter_device_locale/flutter_device_locale_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>

#include <clocale>
#include <cstring>

#define FLUTTER_DEVICE_LOCALE_PLUGIN(obj)                                       \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), flutter_device_locale_plugin_get_type(), \
        FlutterDeviceLocalePlugin))

struct _FlutterDeviceLocalePlugin {
    GObject parent_instance;
};

G_DEFINE_TYPE(FlutterDeviceLocalePlugin, flutter_device_locale_plugin, g_object_get_type())

static const gchar *DEFAULT_LOCALE = "en_US";
static const size_t CATEGORIES_SIZE = 7;
static const int CATEGORIES[CATEGORIES_SIZE] = {
    LC_ALL,
    LC_COLLATE,
    LC_CTYPE,
    LC_MESSAGES,
    LC_MONETARY,
    LC_NUMERIC,
    LC_TIME,
};

// Remove the encoding from the end of the locale string and returns new string
static gchar *remove_encoding(gchar *locale)
{
    if (locale == nullptr) {
        return locale;
    }
    gsize n_bytes = 0;
    for (char *c = locale; c; ++c, n_bytes++) {
        if (*c == '.' || *c == '@') {
            break;
        }
    }
    return g_strndup(locale, n_bytes);
}

static gchar *get_category_locale(int category)
{
    gchar *locale = setlocale(category, "");
    bool no_locale = locale == nullptr
        || strncmp(locale, "LC_", sizeof("LC_") - 1) == 0;
    if (no_locale) {
        g_free(locale);
        return nullptr;
    } else {
        return locale;
    }
}

static FlValue *get_current_locale()
{
    g_autofree gchar *current_locale = nullptr;
    for (int i = 0; i < CATEGORIES_SIZE; i++) {
        current_locale = remove_encoding(get_category_locale(CATEGORIES[i]));
        if (current_locale) {
            break;
        }
    }
    bool use_default = current_locale == nullptr
        || strcmp(current_locale, "C") == 0
        || strcmp(current_locale, "POSIX") == 0;
    if (use_default) {
        return fl_value_new_string(DEFAULT_LOCALE);
    } else {
        return fl_value_new_string(current_locale);
    }
}

static FlMethodResponse *device_locales()
{
    g_autoptr(FlValue) result = fl_value_new_list();
    fl_value_append_take(result, get_current_locale());

    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

// Called when a method call is received from Flutter.
static void flutter_device_locale_plugin_handle_method_call(
    FlutterDeviceLocalePlugin *self,
    FlMethodCall *method_call)
{
    g_autoptr(FlMethodResponse) response = nullptr;

    const gchar *method = fl_method_call_get_name(method_call);

    if (strcmp(method, "deviceLocales") == 0) {
        response = device_locales();
    } else {
        response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
    }

    fl_method_call_respond(method_call, response, nullptr);
}

static void flutter_device_locale_plugin_dispose(GObject *object)
{
    G_OBJECT_CLASS(flutter_device_locale_plugin_parent_class)->dispose(object);
}

static void flutter_device_locale_plugin_class_init(FlutterDeviceLocalePluginClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = flutter_device_locale_plugin_dispose;
}

static void flutter_device_locale_plugin_init(FlutterDeviceLocalePlugin *self) { }

static void method_call_cb(FlMethodChannel *channel, FlMethodCall *method_call,
    gpointer user_data)
{
    FlutterDeviceLocalePlugin *plugin = FLUTTER_DEVICE_LOCALE_PLUGIN(user_data);
    flutter_device_locale_plugin_handle_method_call(plugin, method_call);
}

void flutter_device_locale_plugin_register_with_registrar(FlPluginRegistrar *registrar)
{
    FlutterDeviceLocalePlugin *plugin = FLUTTER_DEVICE_LOCALE_PLUGIN(
        g_object_new(flutter_device_locale_plugin_get_type(), nullptr));

    g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
    g_autoptr(FlMethodChannel) channel = fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
        "flutter_device_locale",
        FL_METHOD_CODEC(codec));
    fl_method_channel_set_method_call_handler(channel, method_call_cb,
        g_object_ref(plugin),
        g_object_unref);

    g_object_unref(plugin);
}
