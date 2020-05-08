#include "status.h"
#include "libstatus.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUuid>
#include <QFile>

const QString pathWalletRoot("m/44'/60'/0'/0");
// EIP1581 Root Key, the extended key from which any whisper key/encryption key can be derived
const QString pathEip1581("m/43'/60'/1581'");
// BIP44-0 Wallet key, the default wallet key
const QString pathDefaultWallet(pathWalletRoot + "/0");
// EIP1581 Chat Key 0, the default whisper key
const QString pathWhisper(pathEip1581  + "/0'/0");

QString jsonToStr(QJsonObject & obj) {
  QJsonDocument doc(obj);
  return QString::fromUtf8(doc.toJson());
}

QString jsonToStr(QJsonArray & arr) {
  QJsonDocument doc(arr);
  return QString::fromUtf8(doc.toJson());
}

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
    //throw QString("Given address " + address + " is not a valid Ethereum address.");
    return "";
  }

    const QString stripAddress = address.right(40).toLower();
    const QString prefix = "";
    //const keccakHash = Hash.keccak256(prefix + stripAddress).toString('hex').replace(/^0x/i, '');
    const QString keccakHash = QString::fromUtf8(QCryptographicHash::hash((prefix + stripAddress).toUtf8(), QCryptographicHash::Keccak_256));
    QString checksumAddress = "0x";

    for (int i = 0; i < stripAddress.length(); ++i) {
      QString s = keccakHash[i];
      int val = s.toInt(nullptr, 16);
        checksumAddress += val >= 8 ? stripAddress[i].toUpper() : stripAddress[i];
    }

    return checksumAddress;
}

Status::Status(QObject * parent): QObject(parent) {
    SetSignalEventCallback((void *)&Status::statusGoEventCallback);

}

void Status::statusGoEventCallback(const char *event) {
  qInfo() << "::statusGoEventCallback call - event: " << event;
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
    const char * result = MultiAccountGenerateAndDeriveAddresses(doc.toJson().data());
    //qInfo() << "libstatus result: " << result;

    return QString(result);

}

const QString dataDir = "./datadir";

QString Status::prepareDirAndUpdateConfig(QString configString) {
  qInfo() << "::prepareDirAndUpdateConfig call - configString:"
                     << configString;

  QJsonParseError jsonError;
  const QJsonDocument &jsonDoc =
      QJsonDocument::fromJson(configString.toUtf8(), &jsonError);
  if (jsonError.error != QJsonParseError::NoError) {
    qWarning() << jsonError.errorString();
  }

  QVariantMap configJSON = jsonDoc.toVariant().toMap();
  QVariantMap shhextConfig = configJSON["ShhextConfig"].toMap();
  qInfo() << "::startNode configString: " << configJSON;

  int networkId = configJSON["NetworkId"].toInt();
  QString relativeDataDirPath = configJSON["DataDir"].toString();
  if (!relativeDataDirPath.startsWith("/"))
    relativeDataDirPath.prepend("/");

  configJSON["DataDir"] = relativeDataDirPath;
  configJSON["KeyStoreDir"] = "keystore";
  configJSON["LogDir"] = relativeDataDirPath;
  configJSON["LogFile"] = "geth.log";

  shhextConfig["BackupDisabledDataDir"] = relativeDataDirPath;

  configJSON["ShhExtConfig"] = shhextConfig;

  const QJsonDocument &updatedJsonDoc = QJsonDocument::fromVariant(configJSON);
  qInfo() << "::startNode updated configString: "
                    << updatedJsonDoc.toVariant().toMap();
  return QString(updatedJsonDoc.toJson(QJsonDocument::Compact));
}

QString Status::multiAccountStoreDerivedAccounts(QString accountJson, QString password) {
  const char * openAccountsResult = OpenAccounts(QString(dataDir).toUtf8().data());
  qInfo() << "openAccounts result: " << openAccountsResult;

  const char * initKeystoreResult = InitKeystore(QString(dataDir + "/keystore").toUtf8().data());
  qInfo() << "initKeystoreResult: " << initKeystoreResult;
  // ::store-multiaccount
  QJsonObject accountJsonObj = QJsonDocument::fromJson(accountJson.toUtf8()).object();
  QString accountId = accountJsonObj["id"].toString();
  QJsonObject obj;
  obj["accountID"] = accountId;
  QJsonArray pathsArr;
  pathsArr.append(pathWalletRoot);
  pathsArr.append(pathEip1581);
  pathsArr.append(pathWhisper);
  pathsArr.append(pathDefaultWallet);
  obj["paths"] = pathsArr;


  qInfo() << "## before hash: ";
  QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));
  obj["password"] = hashedPassword;
  QJsonDocument doc(obj);
  qInfo() << "doc: " << QString::fromUtf8(doc.toJson());
  qInfo() << "## before MultiAccountStoreDerivedAccounts: ";
  const char * result = MultiAccountStoreDerivedAccounts(doc.toJson().data());

  QJsonObject resultObj = QJsonDocument::fromJson(QString(result).toUtf8()).object();

  qInfo() << "## after MultiAccountStoreDerivedAccounts: " << result;
  // ::store-multiaccount callback
  QJsonObject pathWhisperObj = resultObj.value(pathWhisper).toObject();
  QString publicKey = pathWhisperObj.value("publicKey").toString();
  QString alias = generateAlias(publicKey);
  qInfo() << "## after generateAlias: " << alias;
  QString identicon = generateIdenticon(publicKey);
  qInfo() << "## after generateIdenticon: " << identicon;

  pathWhisperObj["name"] = alias;
  pathWhisperObj["photo-path"] = identicon;

  // ::store-multiaccount-success
  accountJsonObj["derived"] = resultObj;
  QString signingPhrase = "bake ball band"; // TODO
  bool saveMnemonic = true;
  QString randomGuid = QUuid::createUuid().toString();
  qInfo() << "## after createUuid: " << randomGuid;

  // on-multiaccount-created
  // 1. prepare-accounts-data

  QJsonObject pathWalletObj = resultObj.value(pathDefaultWallet).toObject();
  QJsonObject walletAccount;
  walletAccount["public-key"] = pathWalletObj["publicKey"].toString();
  walletAccount["address"] = addressToChecksum(pathWalletObj["address"].toString());
  walletAccount["color"] = "blue";
  walletAccount["wallet"] = true;
  walletAccount["path"] = pathDefaultWallet;
  walletAccount["name"] = "Status account";

  QJsonObject whisperAccount;
  whisperAccount["public-key"] = pathWhisperObj["publicKey"].toString();
  whisperAccount["address"] = addressToChecksum(pathWhisperObj["address"].toString());
  whisperAccount["name"] = pathWhisperObj["name"].toString();
  whisperAccount["photo-path"] = pathWhisperObj["photoPath"].toString();
  whisperAccount["path"] = pathWhisper;
  whisperAccount["chat"] = true;

  QJsonArray accountsData;
  accountsData.append(walletAccount);
  accountsData.append(whisperAccount);

  // 2. let bindings

  QJsonObject multiaccData;
  multiaccData["name"] = alias;
  multiaccData["address"] = accountJsonObj["address"].toString();
  multiaccData["photo-path"] = identicon;
  multiaccData["key-uid"] = accountJsonObj["keyUid"].toString();
  QString eip1581Addr = resultObj.value(pathEip1581).toObject().value("address").toString();

  // 2.1 new-multiaccount
  QJsonObject newMultiacc;
  newMultiacc["address"] = accountJsonObj["address"].toString();
  newMultiacc["key-uid"] = accountJsonObj["keyUid"].toString();
  newMultiacc["wallet-root-address"] = resultObj[pathWalletRoot].toObject()["address"].toString();
  newMultiacc["name"] = alias;
  newMultiacc["photo-path"] = identicon;
  newMultiacc["public-key"] = whisperAccount["publicKey"].toString();
  newMultiacc["dapps-address"] = walletAccount["address"].toString();
  newMultiacc["latest-derived-path"] = 0;
  newMultiacc["signing-phrase"] = signingPhrase;
  newMultiacc["installation-id"] = randomGuid;

  // constants/default-multiaccount
  newMultiacc["preview-privacy?"] = true;
  newMultiacc["wallet/visible-tokens"] = "";//"{ mainnet: [\"SNT\"] }";
  newMultiacc["currency"] = "usd";
  newMultiacc["appearance"] = 0;
  newMultiacc["waku-enabled"] = true;
  newMultiacc["log-level"] = "info";

  if (!eip1581Addr.isEmpty()) {
    newMultiacc["eip1581-address"] = eip1581Addr;
  }
  if (saveMnemonic) {
    newMultiacc["mnemonic"] = accountJsonObj["mnemonic"].toString();
  }

  // 2.2 settings
  QJsonObject settings(newMultiacc);
  settings["networks/current-network"] = "mainnet_rpc";

  QFile fDefaultNetworks(":/resources/defaultNetworks.json");
  fDefaultNetworks.open(QIODevice::ReadOnly);
  QJsonArray defaultNetworksJson = QJsonDocument::fromJson(fDefaultNetworks.readAll()).array();
  settings["networks/networks"] = defaultNetworksJson;

  QFile fNodeConfig(":/resources/nodeConfig.json");
  fNodeConfig.open(QIODevice::ReadOnly);
  QString nodeConfigJson = QString::fromUtf8(fNodeConfig.readAll());

  qInfo() << "## before saveAccountAndLogin ";
  return saveAccountAndLogin(jsonToStr(multiaccData), password,
      jsonToStr(settings),
      nodeConfigJson,
      jsonToStr(accountsData));
}

QString Status::saveAccountAndLogin(QString multiaccData,
                                          QString password,
                                          QString settings,
                                          QString nodeConfig,
                                          QString accountsData) {

  QString hashedPassword = QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Keccak_256));
  //QString hashedPassword = "0x2cd9bf92c5e20b1b410f5ace94d963a96e89156fbe65b70365e8596b37f1f165";
  qInfo() << "## in saveAccountAndLogin multiaccData: " << multiaccData;
  qInfo() << "## in saveAccountAndLogin settings: " << settings;
  qInfo() << "## in saveAccountAndLogin nodeConfig: " << nodeConfig;
  qInfo() << "## in saveAccountAndLogin accountsData: " << accountsData;

  QString finalConfig = prepareDirAndUpdateConfig(nodeConfig);

  const char * result = SaveAccountAndLogin(multiaccData.toUtf8().data(),
                                          hashedPassword.toUtf8().data(),
                                          settings.toUtf8().data(),
                                          finalConfig.toUtf8().data(),
                                          accountsData.toUtf8().data());

  return QString(result);
}

QString Status::generateAlias(QString publicKey) {
  QByteArray ba = publicKey.toUtf8();
  const char * result = GenerateAlias({ba.data(), ba.length()});

  return QString(result);
}

QString Status::generateIdenticon(QString publicKey) {
  QByteArray ba = publicKey.toUtf8();
  const char * result = Identicon({ba.data(), ba.length()});

  return QString(result);
}
