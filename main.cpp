#include <QGuiApplication>
#include <FelgoApplication>

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QDebug>

#include "Managers/AppStateManager.h"
#include "Managers/StorageManager.h"

int main(int argc, char *argv[])
{
    // Felgo apps render purely with Qt Quick — pulling QApplication in
    // from QtWidgets adds platform integration code we never use and is
    // a known source of Android startup issues. QGuiApplication is the
    // right pick for any Quick-only app.
    QGuiApplication app(argc, argv);

    FelgoApplication felgo;

    QQmlApplicationEngine engine;
    felgo.initialize(&engine);

    // Make any QML loading failure loud. Without this, an asset/import
    // mistake on Android shows up as an empty white screen with no clue
    // in `adb logcat`. With these handlers wired in we get the offending
    // file:line and message printed to logcat verbatim, which is what
    // you need to debug the device-only crash.
    QObject::connect(
        &engine, &QQmlApplicationEngine::warnings, &app,
        [](const QList<QQmlError>& warnings) {
            for (const QQmlError& w : warnings)
                qCritical().noquote() << "QML warning:" << w.toString();
        });
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [](const QUrl& url) {
            qCritical().noquote() << "QML object creation failed for:" << url.toString();
        });

    // Set an optional license key from project file
    // This does not work if using Felgo Developer App, only for Felgo Cloud Builds and local builds
    // felgo.setLicenseKey(PRODUCT_LICENSE_KEY);

    // The state manager must outlive the QML engine but must NOT also be
    // owned by a stack-bound QScopedPointer when its parent is the
    // engine — that would race the engine's child-deletion at shutdown
    // (especially on Android, where Activity teardown can re-order
    // destruction). Parenting the raw pointer to the engine gives a
    // single, well-defined owner.
    QSharedPointer<StorageManager> storageManager
        = QSharedPointer<StorageManager>::create();
    AppStateManager *stateManager = new AppStateManager(storageManager, &engine);

    if (!stateManager->init()) {
        qWarning() << "AppStateManager::init() returned false; continuing with defaults.";
    }

    engine.rootContext()->setContextProperty(
        QStringLiteral("appStateManager"),
        stateManager);

    felgo.setMainQmlFileName(QStringLiteral("qrc:/qml/Main.qml"));
    engine.load(QUrl(felgo.mainQmlFileName()));

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML root object — see warnings above.";
        return -1;
    }

    return app.exec();
}
