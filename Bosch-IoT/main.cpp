
#include <iostream>

#include <QtCore/QCoreApplication>
#include <QNetworkReply>
#include <qjsondocument.h>
#include <qjsonobject.h>

#include <qtimer.h>

#include "Connector.h"

// Forward declaration
void __async__createThing(Connector& connector);
void __async__listAllAvailableThing(Connector& connector, const QString& thingId);
void __async__deleteThing(Connector& connector, const QString& thingId);
void writeErrorIfDetected(const QNetworkReply& reply, const QJsonDocument& document);

// Create thing and start the async call in the connector
void __async__createThing(Connector& connector) {
  // Create a new thing
  QJsonDocument thing = QJsonDocument::fromJson(R"(
                              {
                                "attributes": {
                                  "dimensions": {
                                    "width": 123,
                                    "height" : 321	}
                                },
                                "features" : {
                                  "dimensions": {
                                    "scalable": true
                                  }
                                }
                              }
                              )");

  auto callback = [](Connector& c, const QNetworkReply& reply, const QJsonDocument& document) {
    std::cout << "Request processed: \t create a new thing" << std::endl;
    std::cout << "HTTP status code: \t " << reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << std::endl;
    QString thingId = document.object()["thingId"].toString();
    std::cout << "Thing ID: \t\t " << thingId.toStdString() << std::endl;
    std::cout << "Content: \t" << document.toJson().toStdString().c_str() << std::endl;
    writeErrorIfDetected(reply, document);
    std::cout << std::endl;

    // next step, list all available thing
    __async__listAllAvailableThing(c, { thingId });
  };

  connector.createThing(thing, callback);
}

// List all available thing and start the async call in the connector
void __async__listAllAvailableThing(Connector& connector, const QString& thingId) {
  auto callback = [thingId](Connector& c, const QNetworkReply& reply, const QJsonDocument& document) {
    std::cout << "Request processed: \t list all available things" << std::endl;
    std::cout << "HTTP status code: \t " << reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << std::endl;
    std::cout << "Content: \t" << document.toJson().toStdString().c_str() << std::endl;
    writeErrorIfDetected(reply, document);
    std::cout << std::endl;

    __async__deleteThing(c, thingId);
  };

  connector.listAllAvailableThings({ thingId }, callback);
}

// Delete created thing and start the async call in the connector
void __async__deleteThing(Connector& connector, const QString& thingId) {
  auto callback = [](Connector& c, const QNetworkReply& reply, const QJsonDocument& document) {
    std::cout << "Request processed: \t delete thing" << std::endl;
    std::cout << "HTTP status code: \t " << reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() << std::endl;
    std::cout << "Content: \t" << document.toJson().toStdString().c_str() << std::endl;
    writeErrorIfDetected(reply, document);
    std::cout << std::endl;

    c.close();
  };

  connector.deleteThing(thingId, callback);
}

void writeErrorIfDetected(const QNetworkReply& reply, const QJsonDocument& document) {
  if (reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 400) {
    return;
  }

  QJsonObject obj = document.object();
  QStringList keys = obj.keys();
  std::cerr << std::endl;
  for (const auto& key : keys) {
    std::cerr << "  " << key.toStdString() << ": " << obj.value(key).toString().toStdString() << std::endl;
  }
}

// Entry point of the application
int main(int argc, char *argv[]) {
  // QCoreApplication is needed for Qt class usage and the event loop to handle signal & slots
  QCoreApplication a(argc, argv);

  // Configure Bosch IoT REST API settings
  Connector::CInfo ci;
  // Set base URL to Rest API -> without a slash at the end!
  ci.m_baseUrl = QStringLiteral("https://things.apps.bosch-iot-cloud.com/api/1");
  // Set API token
  ci.m_api_token = "6804e1c26a49426c8e4d22c707ef4f91";
  // Set credentials
  ci.m_username = QStringLiteral("Jakob");
  ci.m_password = QStringLiteral("JakobPw1!");

  // Create a new Bosch IoT Connector to handle network requests & reply
  Connector* connector = new Connector(ci, &a);

  // Entry point for this example
  // all calls will be executed sequentially and asynchronous to this thread!
  __async__createThing(*connector);

  // This will cause the application to exit when
  // the connector signals finished.    
  QObject::connect(connector, SIGNAL(finished()), &a, SLOT(quit()));

  // Execute the event loop to execute the Connector run method
  return a.exec();
}
