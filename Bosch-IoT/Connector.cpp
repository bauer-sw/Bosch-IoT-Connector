// STL
#include <iostream>

// Qt
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qurlquery.h>
#include <qbuffer.h>

// Own
#include "Connector.h"

Connector::Connector(const CInfo& cinfo, QObject *parent)
  : QObject(parent)
  , m_info(cinfo)
{
  // connect some signal to own defined slots
  connect(&m_networkAccessManager, &QNetworkAccessManager::authenticationRequired, this, &Connector::authenticationRequired);
  connect(&m_networkAccessManager, &QNetworkAccessManager::finished, this, &Connector::replyFinished);
}

Connector::~Connector()
{
}

void Connector::createThing(const QJsonDocument& thing, const Callback& callback) {
  // build API URL
  QUrl url(QStringLiteral("%1/things").arg(m_info.m_baseUrl));

  // convert JSON document to body data
  QBuffer* buffer = new QBuffer();
  buffer->setData(thing.toJson(QJsonDocument::Compact));
  buffer->open(QIODevice::ReadOnly);

  // send request
  sendRequest("POST", url, callback, buffer);
}

void Connector::listAllAvailableThings(const QStringList& ids, const Callback& callback) {
  // build API URL
  QUrl url( QStringLiteral("%1/things").arg( m_info.m_baseUrl ) );

  // update query parameters
  QUrlQuery query;
  QString queryIds;
  for (int i = 0; i < ids.size(); ++i) {
    if (i > 0) {
      queryIds += QStringLiteral(",");
    }

    queryIds += ids[i];
  }
  query.addQueryItem("ids", queryIds);
  url.setQuery(query);
  
  // send request
  sendRequest("GET", url, callback);
}

void Connector::deleteThing(const QString& thingId, const Callback& callback) {
  // build API URL
  QUrl url(QStringLiteral("%1/things/%2").arg(m_info.m_baseUrl, thingId));

  // send request
  sendRequest("DELETE", url, callback);
}

void Connector::sendRequest(const QByteArray& transmissionType, const QUrl& url, const Callback& callback, QIODevice* data) {
  // build a network request
  // set content type as JSON
  // and set the API token
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, QStringLiteral("application/json"));
  request.setRawHeader("x-cr-api-token", m_info.m_api_token);

  // start timer
  m_elapsedTimer.start();

  // send the custom request
  QNetworkReply* reply = m_networkAccessManager.sendCustomRequest(request, transmissionType, data);  

  // added callback to the reply object
  reply->setUserData(0, new CallbackHelper(callback));

  // if data set update the parent to prevent memory access violation
  if (data) {
    data->setParent(reply);
  }
}

void Connector::authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator) {
  // set authenticator
  authenticator->setUser(m_info.m_username);
  authenticator->setPassword(m_info.m_password);
}

void Connector::replyFinished(QNetworkReply* reply) {
  auto elapsed = m_elapsedTimer.elapsed();
  std::cout << " -- " << elapsed << " ms" << std::endl;

  // reply is finished, call callback and delete the object at the next event loop frame
  ((CallbackHelper*)reply->userData(0))->m_callback(*this, *reply, QJsonDocument::fromJson(reply->readAll()));
  reply->deleteLater();
}

void Connector::close() {
  // emit finished to close the application, if signal is connected
  emit finished();
}
