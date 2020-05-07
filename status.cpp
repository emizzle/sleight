#include "status.h"
#include "libstatus.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUuid>

const QString pathWalletRoot("m/44'/60'/0'/0");
// EIP1581 Root Key, the extended key from which any whisper key/encryption key can be derived
const QString pathEip1581("m/43'/60'/1581'");
// BIP44-0 Wallet key, the default wallet key
const QString pathDefaultWallet(pathWalletRoot + "/0");
// EIP1581 Chat Key 0, the default whisper key
const QString pathWhisper(pathEip1581  + "/0'/0");

QString addressToChecksum(QString address) {
/**
 * Converts to a checksum address
 *
 * @method toChecksumAddress
 *
 * @param {string} address the given HEX address
 *
 * @returns {string} address with checksum applied.
 */
  QRegularExpression re("^0x[0-9a-f]{40}$", QRegularExpression::CaseInsensitiveOption);
  if (!re.match(address).hasMatch()) {
    throw Exception("Given address " + address + " is not a valid Ethereum address.");
  }

    const stripAddress = address.right(40).toLower();
    const prefix = "";
    //const keccakHash = Hash.keccak256(prefix + stripAddress).toString('hex').replace(/^0x/i, '');
    const QString keccakHash = QString::fromUtf8(QCryptographicHash::hash((prefix + stripAddress).toUtf8(), QCryptographicHash::Keccak_256));
    QString checksumAddress = "0x";

    for (int i = 0; i < stripAddress.length(); ++i) {
      int val = keccakHash[i].toInt(nullptr, 16);
        checksumAddress += val >= 8 ? stripAddress[i].toUpper() : stripAddress[i];
    }

    return checksumAddress;
}
}

Status::Status(QObject * parent): QObject(parent) {
}

QString Status::multiAccountGenerateAndDeriveAddresses(int n, int mnemonicPhraseLength, QString bip32Passphrase) {
    QJsonObject obj;
    obj["n"] = 5;
    obj["mnemonicPhraseLength"] = 12;
    obj["bip32Passphrase"] = "";
   
    QVector<QString> paths;
    paths.append("m/43'/60'/1581'/0'/0"); 
    paths.append("m/44'/60'/0'/0/0");
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

QString Status::multiAccountStoreDerivedAccounts(QString accountJson, QString password) {
  // ::store-multiaccount
  QJsonObject accountJsonObj = QJsonDocument::fromJson(accountJson).object();
  QString accountId = accountJsonObj["id"];
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

  QJsonDocument resultDoc = QJsonDocument::fromJson(QString(result));

  // ::store-multiaccount callback
  QJsonObject resultObj = resultDoc.object();
  QJsonValue pathWhisperValue = resultObj.value(pathWhisper);
  QJsonValue publicKey = pathWhisperValue.value("publicKey").toString();
  QString alias = generateAlias(publicKey);
  QString identicon = identicon(publicKey);

  pathWhisperValue["name"] = alias;
  pathWhisperValue["photo-path"] = identicon;

  // ::store-multiaccount-success
  accountJsonObj["derived"] = resultObj;
  QString signingPhrase = "bake ball band"; // TODO
  bool saveMnemonic = true;
  QString randomGuid = QUuid::createUuid().toString();

  // on-multiaccount-created
  // 1. prepare-accounts-data
   (let [{:keys [public-key address name photo-path]}
         (get-in multiaccount [:derived constants/path-whisper-keyword])]
     {:public-key public-key
      :address    (eip55/address->checksum address)
      :name       name
      :photo-path photo-path
      :path       constants/path-whisper
      :chat       true})])

  QJsonValue pathWalletValue = resultObj.value(pathDefaultWallet);
  QJsonObject walletAccount;
  walletAccount["public-key"] = pathWalletValue["publicKey"];
  walletAccount["address"] = addressToChecksum(pathWalletValue["address"]);
  walletAccount["color"] = "blue";
  walletAccount["wallet"] = true;
  walletAccount["path"] = pathDefaultWallet;
  walletAccount["name"] = "Status account";

  QJsonObject whisperAccount;
  whisperAccount["public-key"] = pathWhisperValue["publicKey"];
  whisperAccount["address"] = addressToChecksum(pathWhisperValue["address"]);
  whisperAccount["name"] = pathWhisperValue["name"];
  whisperAccount["photo-path"] = pathWhisperValue["photoPath"];
  whisperAccount["path"] = pathWhisper;
  whisperAccount["chat"] = true;

  QJsonArray accountsData;
  accountsData.add(walletAccount);
  accountsData.add(whisperAccount);

  // 2. let bindings

  QJsonObject multiaccData;
  multiaccData["name"] = alias;
  multiaccData["address"] = accountJsonObj["address"];
  multiaccData["photo-path"] = identicon;
  multiaccData["key-uid"] = accountJsonObj["keyUid"];
  QString eip1581Addr = resultObj.value(pathEip1581).value("address");

  // 2.1 new-multiaccount
  QJsonObject newMultiacc;
  newMultiacc["address"] = accountJsonObj["address"];
  newMultiacc["key-uid"] = accountJsonObj["keyUid"];
  newMultiacc["wallet-root-address"] = resultObj[pathWalletRoot]["address"];
  newMultiacc["name"] = alias;
  newMultiacc["photo-path"] = identicon;
  newMultiacc["public-key"] = whisperAccount["publicKey"];
  newMultiacc["dapps-address"] = walletAccount["address"];
  newMultiacc["latest-derived-path"] = 0;
  newMultiacc["signing-phrase"] = signingPhrase;
  newMultiacc["installation-id"] = randomGuid;

  // constants/default-multiaccount
  newMultiacc["preview-privacy?"] = true;
  newMultiacc["wallet/visible-tokens"] = "{ mainnet: [\"SNT\"] }";
  newMultiacc["currency"] = "usd";
  newMultiacc["appearance"] = 0;
  newMultiacc["waku-enabled"] = true;
  newMultiacc["log-level"] = "info";

  if (!eip1581Addr.isEmpty()) {
    newMultiacc["eip1581-address"] = eip1581Addr;
  }
  if (saveMnemonic) {
    newMultiacc["mnemonic"] = accountJsonObj["mnemonic"];
  }

  // 2.2 settings
  QJsonObject settings(newMultiacc);
  settings["networks/current-network"] = "mainnet_rpc";
  settings["networks/networks"] = defaultNetworks;


  saveAccountAndLogin(multiaccData, 
      QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256),
      settings,
      nodeConfig,
      accountsData);


  return resultDoc.toJson();

}

QString Status::generateAlias(QString publicKey) {
  QByteArray ba = publicKey.toUtf8();
  const char * result = GenerateAlias({ba.data(), ba.length()});

  return QString(result);
}

QString Status::identicon(QString publicKey) {
  QByteArray ba = publicKey.toUtf8();
  const char * result = Identicon({ba.data(), ba.length()});

  return QString(result);
}
