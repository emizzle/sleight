#include <QString>
#include <QVector>



class Status {

public:
  static QString multiAccountGenerateAndDeriveAddresses(int n, int mnemonicPhraseLength, QString bip32Passphrase, QVector<QString> paths);

  static QString multiAccountStoreDerivedAccounts(QString accountId, QString hashedPassword);

};
