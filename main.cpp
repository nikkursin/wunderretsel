#include <QApplication>
#include <FelgoApplication>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "Managers/AppStateManager.h"
#include "Managers/StorageManager.h"

// Uncomment this line to add Felgo Hot Reload and use hot reloading with your custom C++ code
//#include <FelgoHotReload>

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    // Work around a Qt 6 Android crash in distance-field glyph cache
    // (QSGDistanceFieldGlyphCache::release/glyphData) that can happen
    // during early Text node updates on scene-graph sync.
    //
    // This keeps behavior unchanged on desktop while forcing the safer
    // text path on Android startup.
    qputenv("QSG_DISTANCEFIELD_DISABLED", "1");
#endif

    QApplication app(argc, argv);

    FelgoApplication felgo;

    QQmlApplicationEngine engine;
    felgo.initialize(&engine);

    // Set an optional license key from project file
    // This does not work if using Felgo Developer App, only for Felgo Cloud Builds and local builds
    // felgo.setLicenseKey(PRODUCT_LICENSE_KEY);

    QSharedPointer<StorageManager> storageManager = QSharedPointer<StorageManager>::create();

    // Owned by `engine` (QObject parent). We deliberately do NOT use a
    // QScopedPointer here because:
    //  - the engine already manages the manager's lifetime via QObject parent,
    //  - QML keeps a raw pointer to it through the context property,
    //  - mixing scoped + parent ownership leads to double-delete / dangling
    //    pointer access during shutdown which presents very similar to the
    //    Android scene-graph crashes we hit at startup.
    AppStateManager *stateManager = new AppStateManager(storageManager, &engine);

    stateManager->init();

    engine.rootContext()->setContextProperty(
        QStringLiteral("appStateManager"),
        stateManager
        );

    felgo.setMainQmlFileName(QStringLiteral("qrc:/qml/Main.qml"));

    engine.load(QUrl(felgo.mainQmlFileName()));

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML";
        return -1;
    }

    // to start your project with Felgo Hot Reload, comment (remove) the lines "felgo.setMainQmlFileName ..." & "engine.load ...",
    // and uncomment the line below
    //FelgoHotReload felgoHotReload(&engine);

    return app.exec();
}
