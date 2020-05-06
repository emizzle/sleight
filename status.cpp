#include "status.h"
#include "libstatus.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

const QString pathWalletRoot("m/44'/60'/0'/0");
// EIP1581 Root Key, the extended key from which any whisper key/encryption key can be derived
const QString pathEip1581("m/43'/60'/1581'");
// BIP44-0 Wallet key, the default wallet key
const QString pathDefaultWallet(pathWalletRoot + "/0");
// EIP1581 Chat Key 0, the default whisper key
const QString pathWhisper(pathEip1581  + "/0'/0");

QString Status::multiAccountGenerateAndDeriveAddresses(int n, int mnemonicPhraseLength, QString bip32Passphrase, QVector<QString> paths) {
    QJsonObject obj;
    obj["n"] = 5;
    obj["mnemonicPhraseLength"] = 12;
    obj["bip32Passphrase"] = "";
    QJsonArray pathsArr;
    QVectorIterator<QString> it(paths);
    while (it.hasNext()) {
      pathsArr.append(it.next());
    }
    obj["paths"] = pathsArr;
    QJsonDocument doc(obj);
    //qInfo() << "doc: " << QString::fromUtf8(doc.toJson());
    const char * result = MultiAccountGenerateAndDeriveAddresses(QString::fromUtf8(doc.toJson()).toUtf8().data());
    //qInfo() << "libstatus result: " << result;

    return QString(result);

}

QString multiAccountStoreDerivedAccounts(QString accountId, QString password) {
  QJsonObject obj;
  obj["accountID"] = accountId;
  QJsonArray pathsArr;
  pathsArr.append(pathWalletRoot);
  pathsArr.append(pathEip1581);
  pathsArr.append(pathWhisper);
  pathsArr.append(pathDefaultWallet);
  obj["paths"] = pathsArr;
  QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));
  obj["password"] = hashedPassword;
  QJsonDocument doc(obj);
  qInfo() << "doc: " << QString::fromUtf8(doc.toJson());
  const char * result = MultiAccountStoreDerivedAccounts(QString::fromUtf8(doc.toJson()).toUtf8().data());
  return QString(result);

}
