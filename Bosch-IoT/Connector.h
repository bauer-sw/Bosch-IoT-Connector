#ifndef CONNECTOR_H
#define CONNECTOR_H

// STL
#include <functional>

// Qt
#include <QObject>
#include <qnetworkaccessmanager.h>
#include <qauthenticator.h>
#include <qelapsedtimer.h>
#include <qurl.h>
#include <QJsonDocument.h>
#include <quuid.h>
#include <qvector.h>

// Class to use to handle the connection to the Bosch IoT cloud system
// REST API v1
// Currently not implemented the full API - for test purpose only
class Connector : public QObject
{
  Q_OBJECT

public:
  // Generic callback definition
  using Callback = std::function<void(Connector&, const QNetworkReply&, const QJsonDocument&)>;

  // Constructor information
  struct CInfo {
    // Bosch IoT base URL
    // e.g. https://things.apps.bosch-iot-cloud.com/api/1
    QString m_baseUrl;

    // Credentials
    QString m_username;
    QString m_password;

    // API Token
    QByteArray m_api_token;
  };

  // Constructor
  Connector(const CInfo& cinfo, QObject *parent = nullptr);

  // Destructor
  ~Connector();

  // Create a new thing
  //    thing: Thing definition as JSON
  // callback: Called after network request is processed
  void createThing(const QJsonDocument& thing, const Callback& callback);

  // List all available things
  //      ids: Ids of the things to show
  // callback: Called after network request is processed
  void listAllAvailableThings(const QStringList& ids, const Callback& callback);

  // Delete thing
  //  thingId: Thing ID to delete
  // callback: Called after network request is processed
  void deleteThing(const QString& thingId, const Callback& callback);
  
  // Close connector -> emit the finished signal to close the application
  void close();

signals:
  // Signal used for to connect with the main thread event loop
  void finished();

private slots:
  // Slot to handle authentication
  void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

  // Slot to handle if request is processed
  void replyFinished(QNetworkReply* reply);

private:
  // Helper to send a request
  //    transmissionType: POST, GET, DELETE ...
  //                 url: URL to be requested for
  //            callback: callback to call after processing
  //                data: body data
  void sendRequest(const QByteArray& transmissionType, const QUrl& url, const Callback& callback, QIODevice* data = nullptr);

  // Initial information like cloud URL
  CInfo m_info;

  // async. network access manager to handle network requests
  QNetworkAccessManager m_networkAccessManager;

  // For time measurement
  QElapsedTimer m_elapsedTimer;

  // Helper class to add the callbacks to the reply objects
  struct CallbackHelper : public QObjectUserData {
    CallbackHelper(const Callback& callback)
      : m_callback(callback)
    {}

    Callback m_callback;
  };
};

#endif // CONNECTOR_H
