#ifndef STATUS_H
#define STATUS_H
#include <QString>
#include <QVector>
#include <QObject>



class Status : public QObject {
  Q_OBJECT

public:
  explicit Status(QObject * parent = nullptr);
  ~Status() {}
  Q_INVOKABLE QString multiAccountGenerateAndDeriveAddresses(int n, int mnemonicPhraseLength, QString bip32Passphrase);

  Q_INVOKABLE QString multiAccountStoreDerivedAccounts(QString accountJson, QString password);
  Q_INVOKABLE QString generateAlias(QString publicKey);
  Q_INVOKABLE QString identicon(QString publicKey);

};

#endif // STATUS_H
