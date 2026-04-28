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
    QApplication app(argc, argv);

    FelgoApplication felgo;

    QQmlApplicationEngine engine;
    felgo.initialize(&engine);

    // Set an optional license key from project file
    // This does not work if using Felgo Developer App, only for Felgo Cloud Builds and local builds
    // felgo.setLicenseKey(PRODUCT_LICENSE_KEY);

    QSharedPointer<StorageManager> storageManager = QSharedPointer<StorageManager>::create();
    QScopedPointer<AppStateManager> stateManager(new AppStateManager(storageManager, engine));

    stateManager->init();

    engine.rootContext()->setContextProperty(
        "appStateManager",
        stateManager.data()
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
